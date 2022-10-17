using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Linq;
using System.Threading.Tasks;

namespace AccountServer.DB
{
    [Table("Account")]
    public class AccountDb
    {
        [Key]
        public int AccountDbId { get; set; }
        public string AccountName { get; set; }
        public string Password { get; set; }
    }
}
