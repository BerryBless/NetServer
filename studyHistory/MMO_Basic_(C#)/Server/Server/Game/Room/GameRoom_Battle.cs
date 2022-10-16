using Google.Protobuf;
using Google.Protobuf.Protocol;
using Server.Data;
using System;
using System.Collections.Generic;
using System.Text;

namespace Server.Game
{
    public partial class GameRoom : JobSerializer
    {

        // C_Move패킷핸들러에서 변경할 내용 
        public void HandleMove(Player player, C_Move movePacket)
        {
            if (player == null) return;


            // TODO : 클라가 거짓으로 보냈는지 검증

            PositionInfo movePosInfo = movePacket.PosInfo;
            ObjectInfo info = player.Info;

            // 다른 좌표로 이동할경우, 갈 수 있는지 체크
            if (movePosInfo.PosX != info.PosInfo.PosX ||
                movePosInfo.PosY != info.PosInfo.PosY)
            {
                // 갈수있냐
                if (Map.CanGo(new Vector2Int(movePosInfo.PosX, movePosInfo.PosY)) == false)
                {
                    //없다
                    return;
                }
            }
            // 상태변화
            info.PosInfo.State = movePosInfo.State;
            info.PosInfo.MoveDir = movePosInfo.MoveDir;
            // 플레이어 이동하고 맵에도 처리
            Map.ApplyMove(player, new Vector2Int(movePosInfo.PosX, movePosInfo.PosY));

            // 다른 플레이어 한테도 알려준다
            S_Move resMovePaket = new S_Move();
            resMovePaket.ObjectId = player.Info.ObjectId;
            resMovePaket.PosInfo = movePacket.PosInfo;

            Broadcast(player.CellPos, resMovePaket);

        }
        // C_Skill
        public void HandleSkill(Player player, C_Skill skillPacket)
        {
            if (player == null) return;

            ObjectInfo info = player.Info;
            if (info.PosInfo.State != CreatureState.Idle) return;

            // TODO : 스킬 사용가능한지 검증


            // 통과
            info.PosInfo.State = CreatureState.Skill;

            S_Skill skill = new S_Skill() { Info = new SkillInfo() };
            skill.ObjectId = info.ObjectId;
            skill.Info.SkillId = skillPacket.Info.SkillId; // 유저가 보내준 스킬
            Broadcast(player.CellPos,skill);

            Data.Skill skillData = null;
            if (DataManager.SkillDict.TryGetValue(skill.Info.SkillId, out skillData) == false) return;

            // 스킬 종류에 따른 로직
            switch (skillData.skillType)
            {
                case SkillType.SkillAuto: // 평타
                                          // 데미지 판정
                    Vector2Int skillPos = player.GetFrontCellPos(info.PosInfo.MoveDir);
                    GameObject target = Map.Find(skillPos);
                    if (target != null)
                    {
                        this.Push(target.OnDamaged, player, player.Info.StatInfo.Attack);
                    }
                    break;
                case SkillType.SkillProjectile: // 투사체
                    SpawnProjectile(player, skillData);
                    break;
                case SkillType.SkillNone:
                    break;
                default:
                    return;
            }

        }

    }
}
