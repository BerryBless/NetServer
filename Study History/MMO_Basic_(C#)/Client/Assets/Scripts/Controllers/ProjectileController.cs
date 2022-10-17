using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public class ProjectileController : CreatureController
{
    // 초기화
    protected override void Init()
    {
        State = CreatureState.Moving;
        SetRotation();
        base.Init();
    }
    // 투사체 방향 정하기
    void SetRotation()
    {
        switch (Dir)
        {
            case MoveDir.Up:
                transform.rotation *= Quaternion.Euler(0, 0, 0);
                break;
            case MoveDir.Down:
                transform.rotation *= Quaternion.Euler(0, 0, 180);
                break;
            case MoveDir.Left:
                transform.rotation *= Quaternion.Euler(0, 0, 90);
                break;
            case MoveDir.Right:
                transform.rotation *= Quaternion.Euler(0, 0, -90);
                break;
        }
    }

    // hp바 생성 안해줌
    protected override void AddHpBar()
    {
    }
}
