using System;
using System.Collections.Generic;
using System.Text;
using Server;
using Microsoft.EntityFrameworkCore;
using Server.Game;
using Server.Data;
using Google.Protobuf.Protocol;

namespace Server.DB
{
    public partial class DbTransaction : JobSerializer
    {
        public static DbTransaction Instance { get; } = new DbTransaction();

        // Me (GameRoom) -> You (Db) -> Me (GameRoom)
        public static void SavePlayerStatus_AllInOne(Player player, GameRoom room)
        {
            if (player == null || room == null)
                return;

            // Me (GameRoom)
            PlayerDb playerDb = new PlayerDb();
            playerDb.PlayerDbId = player.PlayerDbId;
            playerDb.Hp = player.Stat.Hp;

            // You
            Instance.Push(() =>
            {
                using (AppDbContext db = new AppDbContext())
                {
                    db.Entry(playerDb).State = EntityState.Unchanged;
                    db.Entry(playerDb).Property(nameof(PlayerDb.Hp)).IsModified = true;
                    bool success = db.SaveChangesEx();
                    if (success)
                    {
                        // Me
                        //room.Push(() => Console.WriteLine($"Hp Saved({playerDb.Hp})"));
                    }
                }
            });
        }
        // Me (GameRoom) -> You (Db) -> Me (GameRoom)
        public static void SavePlayerStatus_Step1(Player player, GameRoom room)
        {
            if (player == null || room == null)
                return;

            // Me (GameRoom)
            PlayerDb playerDb = new PlayerDb();
            playerDb.PlayerDbId = player.PlayerDbId;
            playerDb.Hp = player.Stat.Hp;


            Instance.Push<PlayerDb, GameRoom>(SavePlayerStatus_Step2, playerDb, room);
        }
        // You
        public static void SavePlayerStatus_Step2(PlayerDb playerDb, GameRoom room)
        {
            using (AppDbContext db = new AppDbContext())
            {
                db.Entry(playerDb).State = EntityState.Unchanged;
                db.Entry(playerDb).Property(nameof(PlayerDb.Hp)).IsModified = true;
                bool success = db.SaveChangesEx();
                if (success)
                {

                    room.Push<int>(SavePlayerStatus_Step3, playerDb.Hp);
                }
            }
        }
        // Me
        public static void SavePlayerStatus_Step3(int Hp)
        {
            Console.WriteLine($"Hp Saved({Hp})");
        }
        // 보상
        public static void RewardPlayer(Player player, RewardData rewardData, GameRoom room)
        {
            if (player == null || rewardData == null || room == null)
                return;

            // TODO 잡큐에 2번들어가는데.. 문제
            int? slot = player.Inven.GetEmptySlot();
            if (slot == null) return;
            // Me (GameRoom)
            ItemDb itemDb = new ItemDb()
            {
                TemplateId = rewardData.itemId,
                Count = rewardData.count,
                Slot = slot.Value,
                OwnerDbId = player.PlayerDbId
            };


            // You
            Instance.Push(() =>
            {
                using (AppDbContext db = new AppDbContext())
                {

                    db.Items.Add(itemDb);
                    bool success = db.SaveChangesEx();
                    if (success)
                    {
                        // Me
                        room.Push(() =>
                                {
                                    Item newItem = Item.MakeItem(itemDb);
                                    player.Inven.Add(newItem);

                                    //  클라이언트  Noti
                                    {
                                        S_AddItem itemPacket = new S_AddItem();
                                        ItemInfo itemInfo = new ItemInfo();
                                        itemInfo.MergeFrom(newItem.info);
                                        itemPacket.Items.Add(itemInfo);

                                        player.Session.Send(itemPacket);
                                    }
                        });
                    }
                }
            });
        }
    }
}
