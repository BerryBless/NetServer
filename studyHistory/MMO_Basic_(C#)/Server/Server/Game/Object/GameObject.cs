using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using System.Text;

namespace Server.Game
{

    public class GameObject
    {
        public GameObjectType ObjectType { get; protected set; } = GameObjectType.None;

        public int Id { get { return Info.ObjectId; } set { Info.ObjectId = value; } }

        public GameRoom Room { get; set; }

        public ObjectInfo Info { get; set; } = new ObjectInfo();
        public PositionInfo PosInfo { get; private set; } = new PositionInfo();
        public StatInfo Stat { get; private set; } = new StatInfo();

        // 추가 스텟 계산을 위함
        public virtual int TotalAttack { get { return Stat.Attack; } }
        public virtual int TotalDefence { get { return 0; } }

        public float Speed
        {
            get { return Stat.Speed; }
            set { Stat.Speed = value; }
        }

        public int HP
        {
            get { return Stat.Hp; }
            set { Stat.Hp = Math.Clamp( value,0,Stat.MaxHp); }
        }

        public CreatureState State
        {
            get { return PosInfo.State; }
            set { PosInfo.State = value; }
        }
        public MoveDir Dir
        {
            get { return PosInfo.MoveDir; }
            set { PosInfo.MoveDir = value; }
        }
        // Dir 벡터에 따른 방향 정하기
        public static MoveDir GetDirFromVector(Vector2Int dir)
        {
            if (dir.x > 0)
                return MoveDir.Right;
            else if (dir.x < 0)
                return MoveDir.Left;
            else if (dir.y > 0)
                return MoveDir.Up;
            return MoveDir.Down;
        }

        public GameObject()
        {
            Info.PosInfo = PosInfo;
            Info.StatInfo = Stat;
            Info.StatInfo.Hp = Stat.MaxHp;
        }

        public Vector2Int CellPos
        {
            get
            {
                return new Vector2Int(PosInfo.PosX, PosInfo.PosY);
            }
            set
            {
                Info.PosInfo.PosX = value.x;
                Info.PosInfo.PosY = value.y;
            }
        }

        public virtual void Update()
        {

        }

        public Vector2Int GetFrontCellPos()
        {
            return GetFrontCellPos(PosInfo.MoveDir);
        }

        // dir방향으로 한칸앞
        public Vector2Int GetFrontCellPos(MoveDir dir)
        {
            Vector2Int frontPos = CellPos;

            switch (dir)
            {
                case MoveDir.Up:
                    frontPos += Vector2Int.up;
                    break;
                case MoveDir.Down:
                    frontPos += Vector2Int.down;
                    break;
                case MoveDir.Left:
                    frontPos += Vector2Int.left;
                    break;
                case MoveDir.Right:
                    frontPos += Vector2Int.right;
                    break;
            }
            return frontPos;
        }
        // 데미지 처리 (피해자시점)
        public virtual void OnDamaged(GameObject attecker, int damage)
        {
            // 스텟에서 HP불러와서 damage만큼 깎아주기
            damage = Math.Max(damage - TotalDefence, 0);
            Stat.Hp = Math.Max(Stat.Hp - damage, 0);

            // 깎인체력을 룸안에 브로드캐스팅
            S_ChangeHp changePacket = new S_ChangeHp();
            changePacket.ObjectId = Id;
            changePacket.Hp = Stat.Hp;
            Room.Broadcast(CellPos,changePacket);

            if(Stat.Hp <= 0)
            {
                OnDead(attecker);
            }
        }
        // 오브젝트 소멸
        public virtual void OnDead(GameObject attecker)
        {
            if (this.Room == null) return;

            S_Die diePacket = new S_Die();
            diePacket.ObjectId = Id; // 내가 죽음
            diePacket.AttackerId = attecker.Id; // 저놈이 나죽임

            this.Room.Broadcast(CellPos, diePacket); // Id가 죽었대!!

            // 리스폰
            GameRoom room = this.Room;
            room.LeaveGame(Id);

            Stat.Hp = Stat.MaxHp;
            PosInfo.State = CreatureState.Idle;
            PosInfo.MoveDir= MoveDir.Down;

            room.EnterGame(this, randomPos : true);
            
        }

        public virtual GameObject GetOwner()
        {
            return this;
        }
    }

}
