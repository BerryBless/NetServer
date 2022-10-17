using System;
using System.Collections.Generic;
using Google.Protobuf.Protocol;
using System.Text;

namespace Server.Game
{
    class Arrow : Projectile
    {
        public GameObject Owner { get; set; }

        public override void Update()
        {
            // 유효성 검사
            if (Data == null || Data.projectile == null || Owner == null || Room == null) return;

            int tick = (int)(1000 / Data.projectile.speed); // 1초에 speed칸만큼 움직임
            Room.PushAfter(tick, Update);// 자기 자신을 예약

            // TODO 앞으로 나가기 / 뿌려주기
            Vector2Int destPos = GetFrontCellPos();
            if (Room.Map.ApplyMove(this, destPos, collision: false) == true)
            {
                S_Move movePacket = new S_Move();
                movePacket.ObjectId = Id;
                movePacket.PosInfo = PosInfo;
                Room.Broadcast(CellPos, movePacket);

                //Console.WriteLine("Move Arrow");
            }
            else
            {
                GameObject target = Room.Map.Find(destPos);
                if (target != null)
                {
                    // 피격판정
                    //Console.WriteLine($"{target.Info.Name}  Damaged : {Data.damage}");
                    target.OnDamaged(this, Data.damage + Owner.TotalAttack);
                }
                // 소멸
                //Room.LeaveGame(Id);
                Room.Push(Room.LeaveGame, Id);
            }
        }

        public override GameObject GetOwner()
        {
            return Owner;
        }
    }
}
