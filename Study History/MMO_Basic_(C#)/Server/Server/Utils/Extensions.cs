using System;
using System.Collections.Generic;
using System.Text;
using Server.DB;
using SharedDB;

namespace Server
{
    public static class Extensions
    {
        internal static bool SaveChangesEx(this AppDbContext db)
        {
            try
            {
                db.SaveChanges();
                return true;
            }
            catch
            {
                // SaveChanges 실패했을때 예외
                return false;
            }
        }
        internal static bool SaveChangesEx(this SharedDbContext db)
        {
            try
            {
                db.SaveChanges();
                return true;
            }
            catch
            {
                // SaveChanges 실패했을때 예외
                return false;
            }
        }
    }
}
