using AccountServer.DB;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using SharedDB;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace AccountServer.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class AccountController : ControllerBase
    {
        AppDbContext _context;
        SharedDbContext _shared;

        public AccountController(AppDbContext context, SharedDbContext shared)
        {
            _context = context;
            _shared = shared;
        }

        // 계정 생성
        [HttpPost]
        [Route("create")]
        public CreateAccountPacketRes CreateAccount([FromBody] CreateAccountPacketReq req)
        {
            CreateAccountPacketRes res = new CreateAccountPacketRes();

            // ID 겹치는지 확인
            AccountDb account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(a => a.AccountName == req.AccountName)
                                    .FirstOrDefault();

            if (account == null)
            {
                // 가능

                _context.Accounts.Add(new AccountDb()
                {
                    AccountName = req.AccountName,
                    Password = req.Password // TODO Hash로 해주기
                });

                bool success = _context.SaveChangesEx();
                res.CreateOk = success;
            }
            else
            {
                // 불가능
                res.CreateOk = false;
            }
            return res;
        }
        // 로그인
        [HttpPost]
        [Route("login")]
        public LoginAccountPacketRes LoginAccount([FromBody] LoginAccountPacketReq req)
        {
            LoginAccountPacketRes res = new LoginAccountPacketRes();

            AccountDb account = _context.Accounts
                                    .AsNoTracking()
                                    .Where(a => a.AccountName == req.AccountName
                                        && a.Password == req.Password)
                                    .FirstOrDefault();


            if(account == null)
            {
                // 문제있음
                res.LoginOk = false;
            }
            else
            {
                // 로그인
                res.LoginOk = true;

                // 토큰발급
                DateTime expired = DateTime.UtcNow;
                expired.AddSeconds(600);

                TokenDb tokenDb = _shared.Tokens
                    .Where(t => t.AccountDbId == account.AccountDbId)
                    .FirstOrDefault();

                if(tokenDb != null)
                {
                    // 토큰갱신
                    tokenDb.Token = new Random().Next(Int32.MinValue, Int32.MaxValue);
                    tokenDb.Expired = expired;
                    _shared.SaveChangesEx();
                }
                else
                {
                    // 토큰생성
                    tokenDb = new TokenDb()
                    {
                        AccountDbId = account.AccountDbId,
                        Token = new Random().Next(Int32.MinValue, Int32.MaxValue),
                        Expired = expired
                    };
                    _shared.Add(tokenDb);
                    _shared.SaveChangesEx();
                }
                res.AccountId = account.AccountDbId;
                res.Token = tokenDb.Token;
                res.ServerList = new List<ServerInfo>();

                // 서버목록
                foreach(ServerDb serverDb in _shared.Servers)
                {
                    res.ServerList.Add(new ServerInfo()
                    {
                        Name = serverDb.Name,
                        IpAddress = serverDb.IpAddress,
                        Port = serverDb.Port,
                        BusyScore = serverDb.BusyScore
                    }) ;
                }
            }

            return res;
        }
    }
}
