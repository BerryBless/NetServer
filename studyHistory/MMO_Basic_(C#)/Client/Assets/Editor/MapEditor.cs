using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Tilemaps;
using System.IO;

// if 안의 코드는 유니티 에디터에서만 사용 가능!
#if UNITY_EDITOR
using UnityEditor;
#endif

public class MapEditor : MonoBehaviour
{
#if UNITY_EDITOR
    // 단축키 %(Ctrl), #(Shift), &(Alt)
    [MenuItem("Tools/GenerateMap %#g")] // Ctrl + Shift + g
    private static void GenerateMap()
    {
        GenerateMap("Assets/Resources/Map");
        GenerateMap("../Common/MapData");

    }

    private static void GenerateMap(string pathPrefix) 
    {
        // 찾기
        GameObject[] gameObjects = Resources.LoadAll<GameObject>("Prefabs/Map");

        // 순회
        foreach (GameObject go in gameObjects)
        {
            // 타일맵 콜리션 불러오기
            Tilemap tmBase = Util.FindChild<Tilemap>(go, "Tilemap_Base", true); // 맵전체 크기정보
            Tilemap tm = Util.FindChild<Tilemap>(go, "Tilemap_Collision", true); // 충돌체크 정보가있는 콜리션


            // 추출한거 출력하기
            using (var writer = File.CreateText( $"{pathPrefix}/{go.name}.txt"))
            {
                writer.WriteLine(tmBase.cellBounds.xMin);
                writer.WriteLine(tmBase.cellBounds.xMax);
                writer.WriteLine(tmBase.cellBounds.yMin);
                writer.WriteLine(tmBase.cellBounds.yMax);

                // 위에서 아래로
                for (int y = tmBase.cellBounds.yMax; y >= tmBase.cellBounds.yMin; y--)
                {
                    // 왼쪽에서 오른쪽으로
                    for (int x = tmBase.cellBounds.xMin; x <= tmBase.cellBounds.xMax; x++)
                    {
                        // 추출해서
                        TileBase tile = tm.GetTile(new Vector3Int(x, y, 0));
                        // 충돌되는 물체면 1
                        if (tile != null) { writer.Write(1); }
                        // 없으면 0
                        else { writer.Write(0); }
                    }
                    writer.WriteLine();
                }
            }

        }
    }
#endif
}
