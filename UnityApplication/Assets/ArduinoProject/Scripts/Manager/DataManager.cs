using System.Collections.Generic;
using UnityEngine;
using System.IO;

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
            jsonPath = Path.Combine(Application.persistentDataPath, "deviceData.json");
            LoadData();
        }
        else
        {
            Destroy(gameObject);
        }
    }

    public void LoadData()
    {
        if (File.Exists(jsonPath))
        {
            Debug.Log("📁 JSON 파일에서 데이터 로딩 중...");
            string json = File.ReadAllText(jsonPath);
            List<DeviceData> dataList = JsonUtilityWrapper.FromJsonList<DeviceData>(json);
            deviceData.Clear();
            foreach (var data in dataList)
            {
                deviceData[data.Mac] = data;
            }
        }
        else
        {
            Debug.Log("📄 CSV에서 초기 데이터 로딩 중...");
            LoadFromCSV();
            SaveData();
        }
    }

    public void SaveData()
    {
        var dataList = new List<DeviceData>(deviceData.Values);
        string json = JsonUtilityWrapper.ToJsonList(dataList, true);
        File.WriteAllText(jsonPath, json);
        Debug.Log("✅ JSON 저장 완료!");
    }

    private void LoadFromCSV()
    {
        var csv = Resources.Load<TextAsset>("DB_Device");
        if (csv == null)
        {
            Debug.LogError("⚠️ CSV 파일을 찾을 수 없습니다.");
            return;
        }

        string[] lines = csv.text.Split('\n');
        if (lines.Length <= 1) return;

        string[] keys = lines[0].Split(',');

        for (int i = 1; i < lines.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(lines[i])) continue;

            string[] values = lines[i].Split(',');

            var data = new DeviceData
            {
                Mac = values[0].Trim(),
                index = int.Parse(values[1]),
                name = values[2].Trim(),
                description = values[3].Trim(),
                autoPaired = values[4].Trim() == "1"
            };

            if (!deviceData.ContainsKey(data.Mac))
            {
                deviceData[data.Mac] = data;
            }
        }
    }
}
