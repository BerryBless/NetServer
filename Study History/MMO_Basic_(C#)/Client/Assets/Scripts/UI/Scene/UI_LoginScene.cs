using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_LoginScene : UI_Scene
{
    enum GameObjects
    {
        AccountName, 
        Password
    }
    enum Images
    {
        CreateBtn,
        LoginBtn
    }
    // 초기화
    public override void Init()
    {
        base.Init();

        Bind<GameObject>(typeof(GameObjects));
        Bind<Image>(typeof(Images));

        GetImage((int)Images.CreateBtn).gameObject.BindEvent(OnClickCreateButton);
        GetImage((int)Images.LoginBtn).gameObject.BindEvent(OnClickLoginButton);
    }

    // 계정생성 버튼 클릭
    public void OnClickCreateButton(PointerEventData evt)
    {
        string account = Get<GameObject>((int)GameObjects.AccountName).GetComponent<InputField>().text;
        string password = Get<GameObject>((int)GameObjects.Password).GetComponent<InputField>().text;

        // 패킷생성
        CreateAccountPacketReq packet = new CreateAccountPacketReq()
        {
            AccountName = account,
            Password = password
        };

        // 계정생성 시도
        Managers.Web.SendPostRequest<CreateAccountPacketRes>("account/create", packet, (res) =>
        {
            Debug.Log($"Account Cteate : {res.CreateOk}");
            Get<GameObject>((int)GameObjects.AccountName).GetComponent<InputField>().text = "";
            Get<GameObject>((int)GameObjects.Password).GetComponent<InputField>().text = "";
        });
    }
    // 로그인 버튼 클릭
    public void OnClickLoginButton(PointerEventData evt)
    {
        string account = Get<GameObject>((int)GameObjects.AccountName).GetComponent<InputField>().text;
        string password = Get<GameObject>((int)GameObjects.Password).GetComponent<InputField>().text;

        // 패킷생성
        LoginAccountPacketReq packet = new LoginAccountPacketReq()
        {
            AccountName = account,
            Password = password
        };

        // 로그인시도
        Managers.Web.SendPostRequest<LoginAccountPacketRes>("account/login", packet, (res) =>
        {
            Debug.Log($"Account Login : {res.LoginOk}");

            Get<GameObject>((int)GameObjects.AccountName).GetComponent<InputField>().text = "";
            Get<GameObject>((int)GameObjects.Password).GetComponent<InputField>().text = "";

            if (res.LoginOk)
            {
                // 토큰(영화표) 들고 있기
                Managers.Network.AccountId = res.AccountId;
                Managers.Network.Token= res.Token;

                // 로그인 성공시 서버목록
                UI_SelectServerPopup popup = Managers.UI.ShowPopupUI<UI_SelectServerPopup>();
                popup.SetServer(res.ServerList);
            }
        });
    }
}
