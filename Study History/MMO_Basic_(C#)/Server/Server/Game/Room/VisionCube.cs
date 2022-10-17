using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Server.Game
{
    public class VisionCube
    {
        // MyPlayer
        public Player Owner { get; private set; }
        // Update호출전 시야에 보이는 오브젝트
        public HashSet<GameObject> PreviousObjects { get; private set; } = new HashSet<GameObject>();

        // Update 추적
        public IJob _job;

        public VisionCube(Player owner)
        {
            Owner = owner;
        }
        public void Clear()
        {
            PreviousObjects.Clear();
        }

        // 호출된 시점의 시야각 내 오브젝트 불러오기
        public HashSet<GameObject> GatherObjects()
        {

            if (Owner == null || Owner.Room == null) return null;
            HashSet<GameObject> objects = new HashSet<GameObject>();

            Vector2Int cellPos = Owner.CellPos;
            List<Zone> zones = Owner.Room.GetAdjacentZones(cellPos);

            // Player
            foreach (Player player in zones.SelectMany(z => z.Players))
            {
                // 시야 범위 계산
                int dy = player.CellPos.y - cellPos.y;
                int dx = player.CellPos.x - cellPos.x;

                // 시야 외는 무시
                if (Math.Abs(dy) > GameRoom.VisionCells) continue;
                if (Math.Abs(dx) > GameRoom.VisionCells) continue;

                // 시야내의 플레이어
                objects.Add(player);
            }

            // Monster
            foreach (Monster monster in zones.SelectMany(z => z.Monsters))
            {
                // 시야 범위 계산
                int dy = monster.CellPos.y - cellPos.y;
                int dx = monster.CellPos.x - cellPos.x;

                // 시야 외는 무시
                if (Math.Abs(dy) > GameRoom.VisionCells) continue;
                if (Math.Abs(dx) > GameRoom.VisionCells) continue;

                // 시야내의 몬스터
                objects.Add(monster);
            }

            // Projectile
            foreach (Projectile projectile in zones.SelectMany(z => z.Projectiles))
            {
                // 시야 범위 계산
                int dy = projectile.CellPos.y - cellPos.y;
                int dx = projectile.CellPos.x - cellPos.x;

                // 시야 외는 무시
                if (Math.Abs(dy) > GameRoom.VisionCells) continue;
                if (Math.Abs(dx) > GameRoom.VisionCells) continue;

                // 시야내의 투사체
                objects.Add(projectile);
            }
            // 시야내의 오브젝트 리스트
            return objects;
        }

        public void Update()
        {
            if (Owner == null || Owner.Room == null)
                return;

            HashSet<GameObject> currentObjects = GatherObjects();

            // 기존에 없었는데 새로 생김
            // currentObjects - PreviousObjects
            // Spawn
            List<GameObject> added = currentObjects.Except(PreviousObjects).ToList();

            // 새로 생긴 것 이 있다면
            if (added.Count > 0)
            {
                // Spawn
                S_Spawn spawnPacket = new S_Spawn();

                foreach (GameObject gameObject in added)
                {
                    ObjectInfo info = new ObjectInfo();
                    info.MergeFrom(gameObject.Info);
                    spawnPacket.Objects.Add(info);
                }

                Owner.Session.Send(spawnPacket);
            }

            // 기존에 있었는데 사라짐
            // PreviousObjects - currentObjects
            // Despawn
            List<GameObject> removed = PreviousObjects.Except(currentObjects).ToList();

            // 사라진 것 이 있다면
            if (removed.Count > 0)
            {
                // Despawn
                S_Despawn despawnPacket = new S_Despawn();

                foreach (GameObject gameObject in removed)
                {
                    despawnPacket.ObjectIds.Add(gameObject.Id);
                }

                Owner.Session.Send(despawnPacket);
            }

            // 갱신
            PreviousObjects = currentObjects;

            // 0.1초뒤에 다시 호출
            _job=Owner.Room.PushAfter(100, Update);
        }

    }
}
