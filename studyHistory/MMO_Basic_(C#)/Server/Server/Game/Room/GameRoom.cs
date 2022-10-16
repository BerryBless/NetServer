using Google.Protobuf;
using Google.Protobuf.Protocol;
using Server.Data;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Server.Game
{
    public partial class GameRoom : JobSerializer
    {
        public const int VisionCells = 5;
        public int RoomId { get; set; }

        Dictionary<int, Player> _players = new Dictionary<int, Player>();               // This.room 안의 플레이어
        Dictionary<int, Monster> _monsters = new Dictionary<int, Monster>();            // This.room 안의 몬스터
        Dictionary<int, Projectile> _projectiles = new Dictionary<int, Projectile>();    // This.room 안의 투사체

        public Zone[,] Zones { get; private set; }
        public int ZoneCells { get; private set; }

        public Map Map { get; private set; } = new Map();

        public Zone GetZone(Vector2Int cellPos)
        {
            int y = (Map.MaxY - cellPos.y) / ZoneCells;
            int x = (cellPos.x - Map.MinX) / ZoneCells;
            return GetZone(y, x);
        }
        public Zone GetZone(int indexY, int indexX)
        {
            if (indexY < 0 || indexY >= Zones.GetLength(0))
                return null;
            if (indexX < 0 || indexX >= Zones.GetLength(1))
                return null;

            return Zones[indexY, indexX];
        }
        public void Init(int mapId, int zoneCells)
        {
            Map.LoadMap(mapId);

            // Zone
            // ㅁㅁㅁ
            // ㅁㅁㅁ
            // ㅁㅁㅁ
            ZoneCells = zoneCells;

            int countY = (Map.SizeY + zoneCells - 1) / zoneCells;
            int countX = (Map.SizeX + zoneCells - 1) / zoneCells;
            Zones = new Zone[countY, countX];
            for (int y = 0; y < countY; y++)
            {
                for (int x = 0; x < countX; x++)
                {
                    Zones[y, x] = new Zone(y, x);
                }
            }

            // TEMP 테스트할몬스터 만들기
            for (int i = 0; i < 20; i++)
            {
                Monster monster = ObjectManager.Instance.Add<Monster>();
                monster.Init(1);
                this.Push(this.EnterGame, monster, true);
            }
        }

        // 누군가 주기적으로 호출해야 겜돌아감
        // JobSerializer 에 넣기
        public void Update()
        {
            Flush();
        }
        // 오브젝트 룸에서 생성해욧
        public void EnterGame(GameObject gameObject, bool randomPos = true)
        {
            if (gameObject == null) return;
            GameObjectType type = ObjectManager.GetObjectTypeById(gameObject.Id);

            if (randomPos)
            {
                Random _rand = new Random();
                Vector2Int respawnPos;
                while (true)
                {
                    respawnPos.x = _rand.Next(Map.MinX, Map.MaxX + 1);
                    respawnPos.y = _rand.Next(Map.MinY, Map.MaxY + 1);

                    if (Map.Find(respawnPos) == null)
                    {
                        gameObject.CellPos = respawnPos;

                        break;
                    }
                }
            }

            // 오브젝트 타입이 Player일때
            if (type == GameObjectType.Player)
            {
                // 플레이어 룸 스폰
                Player player = gameObject as Player;
                // 플레이어가 룸안에 들어옴
                _players.Add(gameObject.Id, player);
                player.Room = this;

                player.RefreshAdditionalStat();

                Map.ApplyMove(player, new Vector2Int(player.CellPos.x, player.CellPos.y));
                GetZone(player.CellPos).Players.Add(player);

                // 클라이언트에게 Room 에서 처리하고 있는 맵을 로드하라!
                {
                    S_ChangeMap changeMapPacket = new S_ChangeMap();
                    changeMapPacket.MapId = this.Map.MapID;
                    player.Session.Send(changeMapPacket);
                }

                // 본인한테 Room 에 있던 player정보 전송
                {
                    S_EnterGame enterPacket = new S_EnterGame();
                    enterPacket.Player = player.Info;
                    player.Session.Send(enterPacket);

                    player.Vision.Update();
                }
            }
            // 오브젝트 타입이 Monster
            else if (type == GameObjectType.Monster)
            {
                // 몬스터 룸 스폰
                Monster monster = gameObject as Monster;
                _monsters.Add(gameObject.Id, monster);
                monster.Room = this;

                GetZone(monster.CellPos).Monsters.Add(monster);
                Map.ApplyMove(monster, new Vector2Int(monster.CellPos.x, monster.CellPos.y));
                // 업데이트 한번실행
                monster.Update();
            }
            // 오브젝트 타입이 Projectile
            else if (type == GameObjectType.Projectile)
            {
                // 투사체 룸 스폰
                Projectile projectile = gameObject as Projectile;
                _projectiles.Add(gameObject.Id, projectile);
                projectile.Room = this;
                GetZone(projectile.CellPos).Projectiles.Add(projectile);
                // 업데이트 한번실행
                projectile.Update();
            }
            // 타인에게 정보 전송
            {
                S_Spawn spawnPacket = new S_Spawn();
                spawnPacket.Objects.Add(gameObject.Info);
                Broadcast(gameObject.CellPos, spawnPacket);
            }
        }
        // Id에 따른 오브젝트 룸에서 지워욧
        public void LeaveGame(int objectId)
        {
            GameObjectType type = ObjectManager.GetObjectTypeById(objectId);

            Vector2Int cellPos;

            if (type == GameObjectType.Player)
            {
                Player player = null;
                if (_players.Remove(objectId, out player) == false)
                    return;

                cellPos = player.CellPos;

                player.OnLeaveGame();
                Map.ApplyLeave(player);
                player.Room = null;


                // 본인한테 정보 전송
                {
                    S_LeaveGame leavePacket = new S_LeaveGame();
                    player.Session.Send(leavePacket);
                }
            }
            else if (type == GameObjectType.Monster)
            {
                Monster monster = null;
                if (_monsters.Remove(objectId, out monster) == false)
                    return;

                cellPos = monster.CellPos;
                Map.ApplyLeave(monster);
                monster.Room = null;
            }
            else if (type == GameObjectType.Projectile)
            {
                Projectile projectile = null;
                if (_projectiles.Remove(objectId, out projectile) == false)
                    return;

                cellPos = projectile.CellPos;
                Map.ApplyLeave(projectile);
                projectile.Room = null;
            }
            else
            {
                return;
            }

            // 타인한테 정보 전송
            {
                S_Despawn despawnPacket = new S_Despawn();
                despawnPacket.ObjectIds.Add(objectId);
                Broadcast(cellPos, despawnPacket);
            }
        }

        public void SpawnProjectile(GameObject owner, Data.Skill skillData)
        {

            if (skillData.projectile.name == "Arrow")
            {
                Arrow arrow = ObjectManager.Instance.Add<Arrow>();

                if (arrow == null)
                    return;
                arrow.Data = skillData;                         // 해당스킬의 정보시트
                arrow.Owner = owner;                           // 주인은 플레이어
                arrow.PosInfo.State = CreatureState.Moving;     // 화살은 계속 움직임
                arrow.PosInfo.PosX = owner.PosInfo.PosX;       // 화살 생성위치
                arrow.PosInfo.PosY = owner.PosInfo.PosY;
                arrow.PosInfo.MoveDir = owner.PosInfo.MoveDir;
                arrow.Speed = skillData.projectile.speed;       // 화살정보 입력
                                                                // 화살 입갤
                this.Push(this.EnterGame, arrow, false);//EnterGame(arrow);
            }
            // else if (skillData.projectile.name == "FireBall") {} // TODO 다른 투사체
        }


        // condition 에따른 플레이어 찾기
        Player FindPlayer(Func<GameObject, bool> condition)
        {
            foreach (Player p in _players.Values)
            {
                if (condition.Invoke(p) == true)
                {
                    return p;
                }
            }
            return null;
        }
        // 가까운 플레이어 찾기
        // A* 플레이어만큼 반복. 그만큼 무겁다는 이야기
        public Player FindClosetPlayer(Vector2Int pos, int range)
        {
            List<Player> players = GetAdjacentPlayers(pos, range);

            // pos와 가까운 순서로 정렬
            players.Sort((left, right) =>
            {
                int leftDist = (left.CellPos - pos).cellDistFromZero;
                int rightDist = (right.CellPos - pos).cellDistFromZero;
                return leftDist - rightDist;
            });

            // 갈 수 있는 가장 가까운 플레이어
            foreach (Player player in players)
            {
                List<Vector2Int> path = Map.FindPath(pos, player.CellPos, checkObjects: true);
                if (path.Count < 2 || path.Count > range)
                    continue;

                return player;
            }

            return null;
        }



        // 같은 Zone에 있는 플레이어 한테 뿌리기
        public void Broadcast(Vector2Int pos, IMessage packet)
        {
            List<Zone> zones = GetAdjacentZones(pos);

            foreach (Player p in zones.SelectMany(z => z.Players))
            {
                int dy = p.CellPos.y - pos.y;
                int dx = p.CellPos.x - pos.x;

                if (Math.Abs(dy) > GameRoom.VisionCells) continue;
                if (Math.Abs(dx) > GameRoom.VisionCells) continue;

                p.Session.Send(packet);
            }
        }

        public List<Zone> GetAdjacentZones(Vector2Int cellPos, int range = GameRoom.VisionCells)
        {
            HashSet<Zone> zones = new HashSet<Zone>();

            int maxY = cellPos.y + range;
            int minY = cellPos.y - range;
            int maxX = cellPos.x + range;
            int minX = cellPos.x - range;

            // 좌측 상단
            Vector2Int leftTop = new Vector2Int(minX, maxY);
            int minIndexY = (Map.MaxY - leftTop.y) / ZoneCells;
            int minIndexX = (leftTop.x - Map.MinX) / ZoneCells;
            // 우측 하단
            Vector2Int rightBot = new Vector2Int(maxX, minY);
            int maxIndexY = (Map.MaxY - rightBot.y) / ZoneCells;
            int maxIndexX = (rightBot.x - Map.MinX) / ZoneCells;

            for (int y = minIndexY; y <= maxIndexY; y++)
                for (int x = minIndexX; x <= maxIndexX; x++)
                {
                    Zone zone = GetZone(y, x);
                    if (zone == null)
                        continue;
                    zones.Add(zone);
                }

            return zones.ToList();
        }
        public List<Player> GetAdjacentPlayers(Vector2Int pos, int range)
        {
            List<Zone> zones = GetAdjacentZones(pos, range);
            return zones.SelectMany(z => z.Players).ToList();
        }
    }
}
