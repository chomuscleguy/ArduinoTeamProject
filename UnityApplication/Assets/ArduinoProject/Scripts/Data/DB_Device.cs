using System.Collections.Generic;
using System;
using UnityEngine;

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
public class DB_Device : ScriptableObjectData
{
    public override void SetDictionaryData()
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

            DataManager.Instance.deviceData.Add(newData.Mac, newData);
        }
    }
}
