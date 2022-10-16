using Google.Protobuf.Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UI_Inventory_Item : UI_Base
{
    // 이미지 바인드
    enum Images
    {
        Background,
        Frame,
        ItemIcon,
    }
    //[SerializeField]
    Image _frame = null;

    //[SerializeField]
    Image _icon = null;


    // 기초적인 정보는 들고있음
    public int ItemDbId { get; private set; }
    public int TemplateId { get; private set; }
    public int Count { get; private set; }
    public bool Equipped { get; private set; }


    public override void Init()
    {
        // 컴포넌트 바인딩
        Bind<Image>(typeof(Images));

        _frame = GetImage((int)Images.Frame);
        _icon = GetImage((int)Images.ItemIcon);

        _frame.gameObject.SetActive(false);
        //_icon.gameObject.SetActive(false);

        // 이벤트핸들러 등록
        _icon.gameObject.BindEvent((e) =>
        {
            // 여기서 아이템 클릭했을때 실행할 코드
            Debug.Log("Click Item");

            // 템플릿ID로 아이템 정보 조회
            Data.ItemData itemData = null;
            Managers.Data.ItemDict.TryGetValue(TemplateId, out itemData);

            if (itemData == null) return;

            // TODO 소무품 아이템 사용!
            if (itemData.itemType == ItemType.Consumable)
                return;

            C_EquipItem equipPacket = new C_EquipItem();
            equipPacket.ItemDbId = ItemDbId;
            equipPacket.Equipped = !Equipped;

            Managers.Network.Send(equipPacket);
        });
    }
    public void SetItem(Item item)
    {
        if (item == null)
        {
            // 아이템 널이면 빈칸
            ItemDbId = 0;
            TemplateId = 0;
            Count = 0;
            Equipped = false;

            _icon.gameObject.SetActive(false);
            _frame.gameObject.SetActive(false);
        }
        else
        {
            // 아이템 데이타 불러와서
            ItemDbId = item.ItemDbId;
            TemplateId = item.TemplateId;
            Count = item.Count;
            Equipped = item.Equipped;
            // 템플릿ID로 아이템 정보 조회
            Data.ItemData itemData = null;
            Managers.Data.ItemDict.TryGetValue(TemplateId, out itemData);

            // 아이콘 이미지 저장하기
            _icon.sprite = Managers.Resource.Load<Sprite>(itemData.iconPath);

            // 착용상테 프레임으로 표시
            _icon.gameObject.SetActive(true);
            _frame.gameObject.SetActive(Equipped);
        }
    }
}