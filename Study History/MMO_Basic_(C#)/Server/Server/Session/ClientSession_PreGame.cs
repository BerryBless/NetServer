using Google.Protobuf.Protocol;
using Microsoft.EntityFrameworkCore;
using Server.Data;
using Server.DB;
using Server.Game;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace Server
{
    public partial class ClientSession : PacketSession
    {
        public int AccountDbId { get; private set; }
        public List<LobbyPlayerInfo> LobbyPlayers { get; set; } = new List<LobbyPlayerInfo>();

        public void HandleLogin(C_Login loginPacket)
        {
            // TODO 보안체크 더 강하게
            if (ServerState != PlayerServerState.ServerStateLogin) return;

            // TODO 각종 상황에 대비
            // -동시?
            // -악질 패킷
            // -이상한 타이밍
            LobbyPlayers.Clear();
            using (AppDbContext db = new AppDbContext())
            {
                AccountDb findAccount = db.Accounts
                    .Include(a => a.Players)
                    .Where(a => a.AccountName == loginPacket.UniqueId)
                    .FirstOrDefault();

                if (findAccount != null)
                { // 로그인 성공
                    AccountDbId = findAccount.AccountDbId;        // Id는 자주쓰니 기억
                    S_Login loginOk = new S_Login() { LoginOk = 1 };
                    foreach (PlayerDb playerDb in findAccount.Players)
                    {
                        LobbyPlayerInfo lobbyPlayer = new LobbyPlayerInfo()
                        {
                            PlayerDbId = playerDb.PlayerDbId,
                            Name = playerDb.PlayerName,
                            StatInfo = new StatInfo()
                            {
                                Level = playerDb.Level,
                                Hp = playerDb.Hp,
                                MaxHp = playerDb.MaxHp,
                                Attack = playerDb.Attack,
                                Speed = playerDb.Speed,
                                TotalExp = playerDb.TotalExp
                            }
                        };

                        // 메모리에 들고있는다 DB접근 최소화
                        LobbyPlayers.Add(lobbyPlayer);
                        // 패킷을 넣어준다
                        loginOk.Players.Add(lobbyPlayer);

                    }
                    Send(loginOk);

                    // 로비로 이동
                    ServerState = PlayerServerState.ServerStateLobby;
                }
                else
                { // 실패
                  // TEMP_ 새계정으로
                    AccountDb newAccount = new AccountDb() { AccountName = loginPacket.UniqueId };
                    db.Accounts.Add(newAccount);
                    bool success = db.SaveChangesEx();
                    if (success == false) return;

                    AccountDbId = newAccount.AccountDbId;        // Id는 자주쓰니 기억

                    // 로그인 실패
                    S_Login loginOk = new S_Login() { LoginOk = 1 };
                    Send(loginOk);

                    // 로비로 이동
                    ServerState = PlayerServerState.ServerStateLobby;
                }
            }
        }

        public void HandleCreatePlayer(C_CreatePlayer createPacket)
        {
            // 로비에서만 가능
            if (ServerState != PlayerServerState.ServerStateLobby) return;

            using (AppDbContext db = new AppDbContext())
            {
                // 찾는거 딱히 의미없음.. 동시에 같은 이름 올 수 도있음
                PlayerDb findPlayer = db.Players
                    .Where(p => p.PlayerName == createPacket.Name)
                    .FirstOrDefault();

                if (findPlayer != null)
                {
                    // 겹치니 못만든다는 뜻으로 null 패킷 보냄
                    Send(new S_CreatePlayer());
                }

                else
                {
                    // 기획상의 1렙 스텟
                    StatInfo stat = null;
                    DataManager.StatDict.TryGetValue(1, out stat);

                    // DB에 플레이어 추가
                    PlayerDb newPlayerDb = new PlayerDb()
                    {
                        AccountId = AccountDbId,
                        PlayerName = createPacket.Name,
                        Level = stat.Level,
                        Hp = stat.Hp,
                        MaxHp = stat.MaxHp,
                        Attack = stat.Attack,
                        Speed = stat.Speed,
                        TotalExp = 0
                    };

                    db.Players.Add(newPlayerDb);
                    bool success = db.SaveChangesEx();
                    if (success == false) return;

                    // 메모리에 추가
                    LobbyPlayerInfo lobbyPlayer = new LobbyPlayerInfo()
                    {
                        PlayerDbId = newPlayerDb.PlayerDbId,
                        Name = newPlayerDb.PlayerName,
                        StatInfo = new StatInfo()
                        {
                            Level = newPlayerDb.Level,
                            Hp = newPlayerDb.Hp,
                            MaxHp = newPlayerDb.MaxHp,
                            Attack = newPlayerDb.Attack,
                            Speed = newPlayerDb.Speed,
                            TotalExp = newPlayerDb.TotalExp
                        }
                    };

                    LobbyPlayers.Add(lobbyPlayer);

                    // 성공 전송
                    S_CreatePlayer newPlayerPacket = new S_CreatePlayer() { Player = new LobbyPlayerInfo() };
                    newPlayerPacket.Player.MergeFrom(lobbyPlayer);
                    Send(newPlayerPacket);
                }
            }
        }

        // 게임입장
        public void HandleEnterGame(C_EnterGame enterGamePacket)
        {
            if (ServerState != PlayerServerState.ServerStateLobby) return;

            LobbyPlayerInfo playerInfo = LobbyPlayers.Find(p => p.Name == enterGamePacket.Name);
            if (playerInfo == null) return;

            // 로비에서 캐릭터 선택완료
            MyPlayer = Game.ObjectManager.Instance.Add<Player>();
            {
                MyPlayer.PlayerDbId = playerInfo.PlayerDbId;
                MyPlayer.Info.Name = playerInfo.Name;
                MyPlayer.Info.PosInfo.State = CreatureState.Idle;
                MyPlayer.Info.PosInfo.MoveDir = MoveDir.Down;
                MyPlayer.Info.PosInfo.PosX = 0;
                MyPlayer.Info.PosInfo.PosY = 0;
                MyPlayer.Stat.MergeFrom(playerInfo.StatInfo);
                MyPlayer.Session = this;

                S_ItemList itemListPacket = new S_ItemList();

                using (AppDbContext db = new AppDbContext())
                {
                    List<ItemDb> items = db.Items
                                        .Where(i => i.OwnerDbId == playerInfo.PlayerDbId)
                                        .ToList();
                    foreach (ItemDb itemDb in items)
                    {
                        Item item = Item.MakeItem(itemDb);
                        if (item != null)
                        {
                            // 인벤에 넣어주기
                            MyPlayer.Inven.Add(item);

                            // 아이템 정보
                            ItemInfo info = new ItemInfo();
                            info.MergeFrom(item.info);
                            itemListPacket.Items.Add(info);
                        }
                    }
                    // 클라에게 아이템 목록보내기
                    Send(itemListPacket);
                }

            }

            ServerState = PlayerServerState.ServerStateGame;
            // TODO 룸번호 바꾸기
            GameLogic.Instance.Push(() =>
            {
                GameRoom room = GameLogic.Instance.Find(1);
                room.Push(room.EnterGame, MyPlayer, true);
            });

        }
    }
}
