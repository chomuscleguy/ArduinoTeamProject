using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System;

public class DataManager : MonoBehaviour
{
    public static DataManager Instance { get; private set; }

    public Dictionary<string, DeviceData> deviceData = new();

    private string jsonPath;

    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);
            jsonPath = Path.Combine(Application.persistentDataPath, "device_data.json");
            LoadData();
        }
        else
        {
            Destroy(gameObject);
        }
    }

    public void LoadData()
    {
        if (!File.Exists(jsonPath))
        {
            Debug.Log("📁 JSON 파일 없음. Resources에서 복사 중...");

            TextAsset jsonAsset = Resources.Load<TextAsset>("DB/DB_Device");
            if (jsonAsset != null)
            {
                File.WriteAllText(jsonPath, jsonAsset.text);
                Debug.Log("✅ 기본 JSON 파일 복사 완료.");
            }
            else
            {
                Debug.LogWarning("⚠️ Resources/DB/DB_Device.json 파일을 찾을 수 없습니다.");
            }
        }

        string json = File.ReadAllText(jsonPath);
        var loadedData = JsonUtility.FromJson<DeviceDataListWrapper>(json);
        deviceData = loadedData.ToDictionary();
    }

    public void SaveData()
    {
        var dataList = new List<DeviceData>(deviceData.Values);
        string json = JsonUtilityWrapper.ToJsonList(dataList, true);
        File.WriteAllText(jsonPath, json);
        Debug.Log("✅ JSON 저장 완료!");
    }
}
