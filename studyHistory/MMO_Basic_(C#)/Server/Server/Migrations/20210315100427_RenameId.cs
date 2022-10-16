using Microsoft.EntityFrameworkCore.Migrations;

namespace Server.Migrations
{
    public partial class RenameId : Migration
    {
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropForeignKey(
                name: "FK_Player_Account_AccountId",
                table: "Player");

            migrationBuilder.DropPrimaryKey(
                name: "PK_Player",
                table: "Player");

            migrationBuilder.DropPrimaryKey(
                name: "PK_Account",
                table: "Account");

            migrationBuilder.DropColumn(
                name: "PlayerId",
                table: "Player");

            migrationBuilder.DropColumn(
                name: "AccountId",
                table: "Account");

            migrationBuilder.AddColumn<int>(
                name: "PlayerDbId",
                table: "Player",
                nullable: false,
                defaultValue: 0)
                .Annotation("SqlServer:Identity", "1, 1");

            migrationBuilder.AddColumn<int>(
                name: "AccountDbId",
                table: "Account",
                nullable: false,
                defaultValue: 0)
                .Annotation("SqlServer:Identity", "1, 1");

            migrationBuilder.AddPrimaryKey(
                name: "PK_Player",
                table: "Player",
                column: "PlayerDbId");

            migrationBuilder.AddPrimaryKey(
                name: "PK_Account",
                table: "Account",
                column: "AccountDbId");

            migrationBuilder.AddForeignKey(
                name: "FK_Player_Account_AccountId",
                table: "Player",
                column: "AccountId",
                principalTable: "Account",
                principalColumn: "AccountDbId",
                onDelete: ReferentialAction.Cascade);
        }

        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropForeignKey(
                name: "FK_Player_Account_AccountId",
                table: "Player");

            migrationBuilder.DropPrimaryKey(
                name: "PK_Player",
                table: "Player");

            migrationBuilder.DropPrimaryKey(
                name: "PK_Account",
                table: "Account");

            migrationBuilder.DropColumn(
                name: "PlayerDbId",
                table: "Player");

            migrationBuilder.DropColumn(
                name: "AccountDbId",
                table: "Account");

            migrationBuilder.AddColumn<int>(
                name: "PlayerId",
                table: "Player",
                type: "int",
                nullable: false,
                defaultValue: 0)
                .Annotation("SqlServer:Identity", "1, 1");

            migrationBuilder.AddColumn<int>(
                name: "AccountId",
                table: "Account",
                type: "int",
                nullable: false,
                defaultValue: 0)
                .Annotation("SqlServer:Identity", "1, 1");

            migrationBuilder.AddPrimaryKey(
                name: "PK_Player",
                table: "Player",
                column: "PlayerId");

            migrationBuilder.AddPrimaryKey(
                name: "PK_Account",
                table: "Account",
                column: "AccountId");

            migrationBuilder.AddForeignKey(
                name: "FK_Player_Account_AccountId",
                table: "Player",
                column: "AccountId",
                principalTable: "Account",
                principalColumn: "AccountId",
                onDelete: ReferentialAction.Cascade);
        }
    }
}
