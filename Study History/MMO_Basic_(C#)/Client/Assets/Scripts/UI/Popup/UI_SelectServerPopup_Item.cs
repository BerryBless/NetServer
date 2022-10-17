using Google.Protobuf.Protocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_SelectServerPopup_Item : UI_Base
{
    // 버튼 바인드
    enum Buttons
    {
        SelectServerButton
    }
    enum Texts
    {
        NameText
    }

    public ServerInfo Info { get; set; }

    // 기초적인 정보는 들고있음
    public int ItemDbId { get; private set; }
    public int TemplateId { get; private set; }
    public int Count { get; private set; }
    public bool Equipped { get; private set; }


    public override void Init()
    {
        // 컴포넌트 바인딩
        Bind<Button>(typeof(Buttons));
        Bind<Text>(typeof(Texts));



        // 이벤트핸들러 등록
        GetButton((int)Buttons.SelectServerButton).gameObject.BindEvent(OnClickButton);
    }

    // 새로고침
    public void RefreshUI()
    {
        if (Info == null) return;
        GetText((int)Texts.NameText).text = Info.Name;
    }
    

    // 클릭이벤트
    public void OnClickButton(PointerEventData evt)
    {
        Managers.Network.ConnectToGame(Info);
        Managers.Scene.LoadScene(Define.Scene.Game);
        Managers.UI.ClosePopupUI();
    }
}