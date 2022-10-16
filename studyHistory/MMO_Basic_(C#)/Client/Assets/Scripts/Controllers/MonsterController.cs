using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;
public class MonsterController : CreatureController
{
    bool _rangedSkill = false;
    protected override void UpdateAnimation()
    {
        if (_animator == null || _sprite == null) return;
        if (State == CreatureState.Idle)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play("IDLE_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play("IDLE_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play("IDLE_RIGHT");
                    _sprite.flipX = true;
                    break;
                default:
                    _animator.Play("IDLE_RIGHT");
                    _sprite.flipX = false;
                    break;
            }
        }
        else if (State == CreatureState.Moving)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play("WALK_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play("WALK_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play("WALK_RIGHT");
                    _sprite.flipX = true; // true일경우 x축을 flip(뒤집다)!
                    break;
                case MoveDir.Right:
                    _animator.Play("WALK_RIGHT");
                    _sprite.flipX = false;
                    break;

            }
        }
        else if (State == CreatureState.Skill)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_BACK" : "ATTACK_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_FRONT" : "ATTACK_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_RIGHT" : "ATTACK_RIGHT");
                    _sprite.flipX = true;
                    break;
                default:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_RIGHT" : "ATTACK_RIGHT");
                    _sprite.flipX = false;
                    break;
            }
        }
        else { }
    }

    protected override void Init()
    {
        base.Init();
    }

    protected override void UpdateIdle()
    {
        base.UpdateIdle();
    }

    public override void OnDamaged()
    {
        // 몬스터 삭제
        //Managers.Object.Remove(Id);
        //Managers.Resource.Destroy(gameObject);
    }
    public override void UseSkill(int skillId)
    {
        if (skillId == 1)
        {
            _rangedSkill = false;
            State = CreatureState.Skill;
        }
        else if (skillId == 2) {
            _rangedSkill = true;
            State = CreatureState.Skill;

        }
    }

}
