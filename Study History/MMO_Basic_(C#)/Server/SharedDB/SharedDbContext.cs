using Microsoft.EntityFrameworkCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharedDB
{
    public class SharedDbContext : DbContext
    {
        public DbSet<TokenDb> Tokens { get; set; }
        public DbSet<ServerDb> Servers { get; set; }

        // GameServer
        public SharedDbContext()
        {
               
        }

        // ASP .NET
        public SharedDbContext (DbContextOptions<SharedDbContext> options) : base(options)
        {

        }

        // GameServer
        public static string ConnectionString { get; set; } = @"Data Source=DESKTOP-OPU51UV\SQLEXPRESS;Initial Catalog=SharedDB;Integrated Security=True;Connect Timeout=30;Encrypt=False;TrustServerCertificate=False;ApplicationIntent=ReadWrite;MultiSubnetFailover=False"; //  ConfigManager.Config.connectionString;
        protected override void OnConfiguring(DbContextOptionsBuilder options)
        {
            if (options.IsConfigured == false)
            {
                options
                    //    .UseLoggerFactory(_logger)
                    .UseSqlServer(ConnectionString);
            }
        }
        protected override void OnModelCreating(ModelBuilder builder)
        {
            builder.Entity<TokenDb>()
                .HasIndex(t => t.AccountDbId)
                .IsUnique();

            builder.Entity<ServerDb>()
                .HasIndex(s => s.Name)
                .IsUnique();
        }

    }
}
