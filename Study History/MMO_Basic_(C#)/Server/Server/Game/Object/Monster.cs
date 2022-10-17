using Google.Protobuf.Protocol;
using Server.Data;
using Server.DB;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices.ComTypes;
using System.Text;

namespace Server.Game
{
    public class Monster : GameObject
    {
        public int TemplateId { get; private set; }

        public Monster()
        {
            ObjectType = GameObjectType.Monster;
        }
        // 데이터 시트로 초기화
        public void Init(int templateId)
        {
            TemplateId = templateId;

            MonsterData monsterData = null;
            DataManager.MonsterDict.TryGetValue(TemplateId, out monsterData);
            Stat.MergeFrom(monsterData.stat);
            Stat.Hp = monsterData.stat.MaxHp;
            State = CreatureState.Idle;
        }

        // 고정 상태 기계 AI
        IJob _job;
        public override void Update()
        {
            switch (State)
            {
                case CreatureState.Idle:
                    UpdateIdle();
                    break;
                case CreatureState.Moving:
                    UpdateMoving();
                    break;
                case CreatureState.Skill:
                    UpdateSkill();
                    break;
                case CreatureState.Dead:
                    UpdateDead();
                    break;
            }

            // 5프레임 (0.2초마다 한번씩 Update)
            if (Room != null)
                _job = Room.PushAfter(200, Update);
        }

        Player _target;
        int _searchCellDist = 10; // 서치 거리
        int _chaseCellDist = 20;// 쫒아가는 거리

        long _nextSearchTick = 0;// 서치 틱카운트
        protected virtual void UpdateIdle()
        {
            if (_nextSearchTick > Environment.TickCount64) return; // 서칭 쿨타임중
            _nextSearchTick = Environment.TickCount64 + 1000;

            Player target = Room.FindClosetPlayer(CellPos, _searchCellDist);

            if (target == null)
                return;

            _target = target;
            State = CreatureState.Moving;
        }

        int _skillRange = 1;
        long _nextMoveTick = 0;
        protected virtual void UpdateMoving()
        {
            if (_nextMoveTick > Environment.TickCount64)
                return;
            int moveTick = (int)(1000 / Speed);
            _nextMoveTick = Environment.TickCount64 + moveTick;

            if (_target == null || _target.Room != Room)
            {
                _target = null;
                State = CreatureState.Idle;
                BroadcastMove();
                return;
            }

            Vector2Int dir = _target.CellPos - CellPos;
            int dist = dir.cellDistFromZero;
            if (dist == 0 || dist > _chaseCellDist)
            {
                _target = null;
                State = CreatureState.Idle;
                BroadcastMove();
                return;
            }

            List<Vector2Int> path = Room.Map.FindPath(CellPos, _target.CellPos, checkObjects: true);
            if (path.Count < 2 || path.Count > _chaseCellDist)
            {
                _target = null;
                State = CreatureState.Idle;
                BroadcastMove();
                return;
            }

            // 스킬로 넘어갈지 체크
            if (dist <= _skillRange && (dir.x == 0 || dir.y == 0))
            {
                _coolTick = 0;
                State = CreatureState.Skill;
                return;
            }

            // 이동
            Dir = GetDirFromVector(path[1] - CellPos);
            Room.Map.ApplyMove(this, path[1]);
            BroadcastMove();
        }

        void BroadcastMove()
        {
            // 다른 플레이어한테도 알려준다
            S_Move movePacket = new S_Move();
            movePacket.ObjectId = Id;
            movePacket.PosInfo = PosInfo;
            Room.Broadcast(CellPos, movePacket);
        }

        long _coolTick = 0;
        protected virtual void UpdateSkill()
        {
            if (_coolTick == 0)
            {
                // 유효한 타겟인지
                if (_target == null || _target.Room != Room)
                {
                    _target = null;
                    State = CreatureState.Moving;
                    BroadcastMove();
                    return;
                }

                // 스킬이 아직 사용 가능한지
                Vector2Int dir = (_target.CellPos - CellPos);
                int dist = dir.cellDistFromZero;
                bool canUseSkill = (dist <= _skillRange && (dir.x == 0 || dir.y == 0));
                if (canUseSkill == false)
                {
                    State = CreatureState.Moving;
                    BroadcastMove();
                    return;
                }

                // 타게팅 방향 주시
                MoveDir lookDir = GetDirFromVector(dir);
                if (Dir != lookDir)
                {
                    Dir = lookDir;
                    BroadcastMove();
                }

                Skill skillData = null;
                DataManager.SkillDict.TryGetValue(1, out skillData);

                // 데미지 판정
                _target.OnDamaged(this, skillData.damage + TotalAttack);

                // 스킬 사용 Broadcast
                S_Skill skill = new S_Skill() { Info = new SkillInfo() };
                skill.ObjectId = Id;
                skill.Info.SkillId = skillData.id;
                Room.Broadcast(CellPos, skill);

                // 스킬 쿨타임 적용
                int coolTick = (int)(1000 * skillData.cooldown);
                _coolTick = Environment.TickCount64 + coolTick;
            }

            if (_coolTick > Environment.TickCount64)
                return;

            _coolTick = 0;
        }

        protected virtual void UpdateDead()
        {

        }

        public override void OnDead(GameObject attacker)
        {
            if (_job != null)
            {
                _job.Cancel = true;
                _job = null;
            }
            base.OnDead(attacker);

            GameObject owner = attacker.GetOwner(); // 막타친거가 플레이어가 아닐 수 도 있음 (여기선 투사체)
            if (owner.ObjectType == GameObjectType.Player)
            {
                RewardData rewardData = GetRandomReward();
                if (rewardData != null)
                {
                    Player player = (Player)owner;
                    DbTransaction.RewardPlayer(player, rewardData, Room);
                }
            }

        }

        RewardData GetRandomReward()
        {
            MonsterData monsterData = null;
            DataManager.MonsterDict.TryGetValue(TemplateId, out monsterData);

            int rand = new Random().Next(0, 101);//0~100

            // 10 | 10 | 10 | 10 | 10 | 50(꽝)
            int sum = 0;
            foreach (RewardData rewardData in monsterData.rewards) // rand = 32
            {
                sum += rewardData.probability; // 40까지 누적

                if (rand <= sum)// 32 <= 40 4번째꺼 당첨
                {
                    return rewardData;
                }
            }

            return null;
        }
    }
}
