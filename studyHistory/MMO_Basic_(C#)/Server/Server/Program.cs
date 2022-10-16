using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Google.Protobuf;
using Google.Protobuf.Protocol;
using Google.Protobuf.WellKnownTypes;
using Server.Game;
using ServerCore;
using Server.Data;
using Server.DB;
using System.Linq;
using SharedDB;

namespace Server
{
    class Program
    {
        static Listener _listener = new Listener();

        static void GameLogicTask()
        {
            while (true)
            {
                GameLogic.Instance.Update();
                Thread.Sleep(0);
            }
        }

        static void DbTask()
        {
            while (true)
            {
                DbTransaction.Instance.Flush();
                Thread.Sleep(0);
            }
        }


        static void NetworkTask()
        {
            while (true)
            {
                // 모든 플레이어를 순회
                List<ClientSession> sessions = SessionManager.Instance.GetSessions();
                foreach (ClientSession session in sessions)
                {
                    // Flush
                    session.FlushSend();
                }
                Thread.Sleep(0);
            }
        }


        //TEMP 서버목록테스트 데이터시트에서 불러와야함
        public static String Name { get; } = "카무라";
        public static int Port { get; } = 7777;
        public static string IpAddress { get; set; }
        static void StartServerInfoTask()
        {
            var t = new System.Timers.Timer();
            t.AutoReset = true;
            t.Elapsed += new System.Timers.ElapsedEventHandler((s, e) =>
            {
                // 현재 서버상태 정보 갱신
                using (SharedDbContext sharedDb = new SharedDbContext())
                {
                    ServerDb serverDb = sharedDb.Servers.Where(s => s.Name == Name).FirstOrDefault();
                    if (serverDb != null)
                    {
                        serverDb.IpAddress = IpAddress;
                        serverDb.Port = Port;
                        serverDb.BusyScore = SessionManager.Instance.GetBusyScore();
                        sharedDb.SaveChangesEx();
                    }
                    else
                    {
                        serverDb = new ServerDb()
                        {
                            Name = Program.Name,
                            IpAddress = Program.IpAddress,
                            Port = Program.Port,
                            BusyScore = SessionManager.Instance.GetBusyScore()
                        };
                        sharedDb.Servers.Add(serverDb);
                        sharedDb.SaveChangesEx();

                    }
                }
            });
            t.Interval = 10 * 1000;
            t.Start();
        }





        static void Main(string[] args)
        {
            // "게임기획" 데이타 불러오기
            ConfigManager.LoadConfig();
            DataManager.LoadData();

            // 룸 생성
            GameLogic.Instance.Push(() =>
            {
                GameLogic.Instance.Add(2);
            });

            // DNS (Domain Name System)
            string host = Dns.GetHostName();
            IPHostEntry ipHost = Dns.GetHostEntry(host);
            IPAddress ipAddr = ipHost.AddressList[0];
            IPEndPoint endPoint = new IPEndPoint(ipAddr, 7777);

            IpAddress = ipAddr.ToString();

            _listener.Init(endPoint, () => { return SessionManager.Instance.Generate(); });
            Console.WriteLine("Listening...");

            // StartServerInfoTask
            StartServerInfoTask();

            // NetworkTask
            {
                //Task networkTask = new Task(NetworkTask, TaskCreationOptions.LongRunning);
                //networkTask.Start();
                Thread t = new Thread(NetworkTask);
                t.Name = "Network Send";
                t.Start();
            }

            // DbTask
            {
                //Task dbTask = new Task(DbTask, TaskCreationOptions.LongRunning);
                //dbTask.Start();
                Thread t = new Thread(DbTask);
                t.Name = "DB";
                t.Start();
            }
            // GameLogicTask
            {
                //Task gameLogicTask = new Task(GameLogicTask, TaskCreationOptions.LongRunning);
                //gameLogicTask.Start();
                Thread.CurrentThread.Name = "GameLogic";
                GameLogicTask();
            }
        }
    }
}
