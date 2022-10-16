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

        // 보상
        public static void EquipItemNoti(Player player, Item item)
        {
            if (player == null || item == null)
                return;

            ItemDb itemDb = new ItemDb()
            {
                ItemDbId = item.ItemDbId,
                Equipped = item.Equipped
            };
            // You
            Instance.Push(() =>
            {
                using (AppDbContext db = new AppDbContext())
                {
                    db.Entry(itemDb).State = EntityState.Unchanged;
                    db.Entry(itemDb).Property(nameof(itemDb.Equipped)).IsModified = true;
                    bool success = db.SaveChangesEx();
                    if (!success)
                    {
                        // TODO 실패처리
                    }
                }
            });
        }
    }
}
