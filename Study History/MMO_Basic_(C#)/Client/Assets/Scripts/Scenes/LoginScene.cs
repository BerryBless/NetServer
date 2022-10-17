using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LoginScene : BaseScene
{
    UI_LoginScene _sceneUI;
    protected override void Init()
    {
        base.Init();

        SceneType = Define.Scene.Login;

        // TEMP Web테스트
        Managers.Web.BaseUrl = "https://localhost:5001/api";

        Screen.SetResolution(640, 480, false);// 테스트용 빌드 해상도 설정

        _sceneUI = Managers.UI.ShowSceneUI<UI_LoginScene>();
    }

    public override void Clear()
    {

    }
}
