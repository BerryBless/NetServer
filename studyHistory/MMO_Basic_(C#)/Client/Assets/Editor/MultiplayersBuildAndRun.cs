using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

public class MultiplayersBuildAndRun
{
    [MenuItem("Tools/Run Multiplayer/2 Players %#t")]
    static void PreformWin64Build2()
    {
        PreformWin64Build(2);
    }
    [MenuItem("Tools/Run Multiplayer/3 Players")]
    static void PreformWin64Build3()
    {
        PreformWin64Build(3);
    }
    [MenuItem("Tools/Run Multiplayer/4 Players")]
    static void PreformWin64Build4()
    {
        PreformWin64Build(4);
    }

    static void PreformWin64Build(int playerCount)
    {
        EditorUserBuildSettings.SwitchActiveBuildTarget(
            BuildTargetGroup.Standalone, BuildTarget.StandaloneWindows);                                                // 빌드 옵션 선택
        for (int i = 1; i <= playerCount; i++)
        {
            BuildPipeline.BuildPlayer(GetScencePaths(),
                "Builds/Win64/" + GetProjectName() + i.ToString() + "/" + GetProjectName() + i.ToString() + ".exe",     // 이 이름으로 빌드하고
                BuildTarget.StandaloneWindows64, BuildOptions.AutoRunPlayer);                                            // 바로 실행해
        }
    }
    static string GetProjectName()
    {
        string[] s = Application.dataPath.Split('/');
        return s[s.Length - 2];
    }
    static string[] GetScencePaths()
    {
        string[] scenes = new string[EditorBuildSettings.scenes.Length];

        for (int i = 0; i < scenes.Length; i++)
        {
            scenes[i] = EditorBuildSettings.scenes[i].path;
        }

        return scenes;
    }
}
