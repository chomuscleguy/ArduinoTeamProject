using System.Collections.Generic;
using System.IO;
using UnityEngine;

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
            BluetoothManager.Instance.Toast("📁 JSON 파일 없음. CSV에서 생성 중...");
            LoadFromCSVAndSave();
        }

        string jsonContent = File.ReadAllText(jsonPath);
        var loadedData = JsonUtility.FromJson<DeviceDataListWrapper>(jsonContent);
        deviceData = loadedData.ToDictionary();
        BluetoothManager.Instance.Toast("✅ JSON 로딩 완료.");
    }

    public void SaveData()
    {
        var dataList = new List<DeviceData>(deviceData.Values);
        var wrapper = new DeviceDataListWrapper { devices = dataList };
        string jsonOutput = JsonUtility.ToJson(wrapper, true);
        File.WriteAllText(jsonPath, jsonOutput);
        BluetoothManager.Instance.Toast("✅ JSON 저장 완료!");
    }

    private void LoadFromCSVAndSave()
    {
        TextAsset csv = Resources.Load<TextAsset>("DB/DB_Device");
        if (csv == null)
        {
            BluetoothManager.Instance.Toast("⚠️ CSV 파일을 찾을 수 없습니다.");
            return;
        }

        string[] lines = csv.text.Split('\n');
        if (lines.Length <= 1) return;

        deviceData.Clear();
        for (int i = 1; i < lines.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(lines[i])) continue;

            string[] values = lines[i].Split(',');
            if (values.Length < 5) continue;

            DeviceData data = new DeviceData
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

        SaveData();
        BluetoothManager.Instance.Toast("✅ CSV로부터 데이터 로드 및 JSON 생성 완료.");
    }
}