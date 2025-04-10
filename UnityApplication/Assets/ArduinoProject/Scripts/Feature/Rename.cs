using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class Rename : MonoBehaviour
{
    public TMP_InputField InputField;
    public Image renameImage;
    public Image saveImage;

    private string macAddress;

    public void SetMacAddress(string mac)
    {
        macAddress = mac;
        if (DataManager.Instance.deviceData.TryGetValue(macAddress, out var data))
        {
            InputField.text = data.name;
        }
    }

    public void Renaming()
    {
        if (DataManager.Instance.deviceData.TryGetValue(macAddress, out var data))
        {
            InputField.interactable = true;
            saveImage.gameObject.SetActive(true);
            renameImage.gameObject.SetActive(false);
            
        }
    }

    public void Save()
    {
        if (DataManager.Instance.deviceData.TryGetValue(macAddress, out var data))
        {
            InputField.interactable = false;
            data.name = InputField.text;
            saveImage.gameObject.SetActive(false);
            renameImage.gameObject.SetActive(true);
            DataManager.Instance.SaveData();
            Debug.Log($"✅ 이름이 '{data.name}'으로 변경되었습니다.");
        }
    }
}
