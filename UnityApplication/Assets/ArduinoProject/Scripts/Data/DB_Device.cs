using UnityEngine;
using System.Collections.Generic;
using System;
using System.IO;

[Serializable]
public class DeviceData
{
    public string Mac;
    public int index;
    public string name;
    public string description;
    public bool autoPaired;

    public override string ToString()
    {
        return $"[{index}] {name} ({Mac}) - {description} | Auto: {autoPaired}";
    }
}

[CreateAssetMenu(fileName = "DB_Device", menuName = "DB/Device")]
public class DB_Device : ScriptableObject
{
    public TextAsset CSV;

    public void LoadFromCSV()
    {
        string[] textLine = CSV.text.Split('\n');
        string[] Keys = textLine[0].Split(',');

        for (int i = 1; i < textLine.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(textLine[i])) continue;

            string[] values = textLine[i].Split(',');
            Dictionary<string, string> keyValues = new Dictionary<string, string>();

            for (int j = 0; j < values.Length; j++)
            {
                if (j >= Keys.Length) continue;
                keyValues[Keys[j]] = values[j].Trim();
            }

            if (!keyValues.ContainsKey("index")) continue;

            DeviceData newData = new DeviceData();
            newData.Mac = keyValues[nameof(newData.Mac)];
            newData.index = int.Parse(keyValues[nameof(newData.index)]);
            newData.name = keyValues[nameof(newData.name)];
            newData.description = keyValues[nameof(newData.description)];
            newData.autoPaired = keyValues[nameof(newData.autoPaired)] == "1";

            DataManager.Instance.deviceData[newData.Mac] = newData;
        }
    }

    public void LoadFromJSON()
    {
        string path = Path.Combine(Application.persistentDataPath, "deviceData.json");
        if (!File.Exists(path))
        {
            Debug.LogWarning("📂 JSON 파일이 존재하지 않습니다. CSV에서 불러옵니다.");
            LoadFromCSV();
            return;
        }

        string json = File.ReadAllText(path);
        DeviceDataListWrapper wrapper = JsonUtility.FromJson<DeviceDataListWrapper>(json);

        DataManager.Instance.deviceData.Clear();
        foreach (var device in wrapper.devices)
        {
            DataManager.Instance.deviceData[device.Mac] = device;
        }
        Debug.Log("📥 JSON 데이터 로드 완료");
    }

    public void SaveToJSON()
    {
        DeviceDataListWrapper wrapper = new DeviceDataListWrapper();
        wrapper.devices = new List<DeviceData>(DataManager.Instance.deviceData.Values);

        string json = JsonUtility.ToJson(wrapper, true);
        string path = Path.Combine(Application.persistentDataPath, "deviceData.json");
        File.WriteAllText(path, json);

        Debug.Log("💾 JSON 저장 완료: " + path);
    }
}
