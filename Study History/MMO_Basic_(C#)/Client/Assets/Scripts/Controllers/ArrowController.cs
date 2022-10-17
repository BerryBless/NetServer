using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;
public class ArrowController : ProjectileController
{
    
    protected override void UpdateAnimation()
    {
        // 한방향으로만 움직이기 때문에 딱히 애니메이션 없음
    }
    /* 화살은 서버에서 관리
    protected override void MoveToNextPos()
    {

        Vector3Int destPos = CellPos;

        switch (Dir)
        {
            case MoveDir.Up:
                destPos += Vector3Int.up;
                break;
            case MoveDir.Down:
                destPos += Vector3Int.down;
                break;
            case MoveDir.Left:
                destPos += Vector3Int.left;
                break;
            case MoveDir.Right:
                destPos += Vector3Int.right;
                break;
        }
        if (Managers.Map.CanGo(CellPos) == true)
        {
            GameObject go = Managers.Object.Find(destPos);
            if (go == null)
            {
                CellPos = destPos;
            }
            else
            {
                CreatureController cc = go.GetComponent<CreatureController>();
                if (cc != null)
                {
                    cc.OnDamaged();
                }
                // 화살삭제
                Managers.Resource.Destroy(gameObject);
            }
        }
        else
        {
            Managers.Resource.Destroy(gameObject);
        }
    }*/

    
}
