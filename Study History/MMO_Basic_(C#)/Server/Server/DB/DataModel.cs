using System;
using System.ComponentModel.DataAnnotations.Schema;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel.DataAnnotations;

namespace Server.DB
{
    [Table("Player")]
    public class PlayerDb{
        [Key]
        public int PlayerDbId { get; set; }
        public string PlayerName { get; set; }

        [ForeignKey("Account")]
        public int AccountId { get; set; }
        public AccountDb Account { get; set; }

        public ICollection<ItemDb> Items { get; set; }

        // 따로 테이블 만들어도됨
        public int Level { get; set; }
        public int Hp { get; set; }
        public int MaxHp { get; set; }
        public int Attack { get; set; }
        public float Speed { get; set; }
        public int TotalExp { get; set; }
    }

    [Table("Account")]
    public class AccountDb
    {
        [Key]
        public int AccountDbId { get; set; }
        public string AccountName { get; set; }

        public ICollection<PlayerDb> Players { get; set; }
    }


    [Table("Item")]
    public class ItemDb
    {
        [Key]
        public int ItemDbId { get; set; }

        public int TemplateId { get; set; }
        public int Count { get; set; }
        public int Slot{ get; set; }
        public bool Equipped { get; set; } = false;

        [ForeignKey("Owner")]
        public int? OwnerDbId { get; set; }
        public PlayerDb Owner { get; set; }
    }

}
