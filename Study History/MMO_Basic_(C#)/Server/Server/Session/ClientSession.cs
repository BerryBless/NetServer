using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using ServerCore;
using Server.Game;
using System.Net;
using Google.Protobuf.Protocol;
using Google.Protobuf;
using Server.Data;

namespace Server
{
    public partial class ClientSession : PacketSession
    {
        public PlayerServerState ServerState { get; private set; } = PlayerServerState.ServerStateLogin;
        public Player MyPlayer { get; set; }
        public int SessionId { get; set; }
        List<ArraySegment<byte>> _reserveQueue = new List<ArraySegment<byte>>();
        object _lock = new object();

        // 패킷 모아보내기
        int _reservedSendBytes = 0; // 보낼려고 예약한 버퍼는 Byte 인지
        long _lastSendTick = 0;// 마지막으로 보낸뒤 시간이 몇틱 지났는 지

        #region NETWORK
        // 클라랑 연결중인지
        long _pingpongTick = 0;
        public void Ping()
        {
            if (_pingpongTick > 0)
            {
                long delta = (System.Environment.TickCount64 - _pingpongTick);

                // 30초 동안 응답이 없으면
                if (delta > 30 * 1000)
                {
                    Console.WriteLine("Disconnected by PingCheck");

                    Disconnect();
                    return;
                }
            }

            S_Ping pingPacket = new S_Ping();
            Send(pingPacket);

            GameLogic.Instance.PushAfter(5000, Ping);
        }
        public void HandlePong()
        {
            // 갱신
            _pingpongTick = System.Environment.TickCount64;
        }

        public void Send(IMessage packet)
        {
            // 패킷 이름을 이용해서 ID를 정함
            string msgName = packet.Descriptor.Name.Replace("_", string.Empty); // S_Chat => SChat
            MsgId msgId = (MsgId)Enum.Parse(typeof(MsgId), msgName);

            ushort size = (ushort)packet.CalculateSize();
            byte[] sendBuffer = new byte[size + 4];
            Array.Copy(BitConverter.GetBytes((ushort)(size + 4)), 0, sendBuffer, 0, sizeof(ushort));
            Array.Copy(BitConverter.GetBytes((ushort)msgId), 0, sendBuffer, 2, sizeof(ushort));
            Array.Copy(packet.ToByteArray(), 0, sendBuffer, 4, size);


            lock (_lock)
            {
                // 예약만하고 보내지는 않음
                _reserveQueue.Add(sendBuffer);
                _reservedSendBytes += sendBuffer.Length;
            }
        }

        // 실제 보내는 부분
        public void FlushSend()
        {
            List<ArraySegment<byte>> sendList = null;
            lock (_lock)
            {
                // 0.1초가 지났거나 패킷이 1만바이트 이상일떄
                long delta = (System.Environment.TickCount64 - _lastSendTick);
                if (delta < 100 && _reservedSendBytes < 10000)
                    return;
                if (_reserveQueue.Count == 0)
                    return;

                // 패킷 모아보내기
                _reservedSendBytes = 0;
                _lastSendTick = System.Environment.TickCount64;

                sendList = _reserveQueue;
                _reserveQueue = new List<ArraySegment<byte>>();
            }

            Send(sendList);
        }

        public override void OnConnected(EndPoint endPoint)
        {
            Console.WriteLine($"OnConnected : {endPoint}");

            {
                S_Connected connectedPacket = new S_Connected();
                Send(connectedPacket);
            }

            GameLogic.Instance.PushAfter(5000, Ping);
        }

        public override void OnRecvPacket(ArraySegment<byte> buffer)
        {
            PacketManager.Instance.OnRecvPacket(this, buffer);
        }

        public override void OnDisconnected(EndPoint endPoint)
        {
            // TEMP 1번룸만 존재
            //RoomManager.Instance.Find(1).LeaveGame(MyPlayer.Info.ObjectId);
            GameLogic.Instance.Push(() =>
            {
                if (MyPlayer == null) return;

                GameRoom room = GameLogic.Instance.Find(1);
                room.Push(room.LeaveGame, MyPlayer.Info.ObjectId);
            });
            SessionManager.Instance.Remove(this);

            //Console.WriteLine($"OnDisconnected : {endPoint}");
        }

        public override void OnSend(int numOfBytes)
        {
            //Console.WriteLine($"Transferred bytes: {numOfBytes}");
        }
        #endregion
    }
}
