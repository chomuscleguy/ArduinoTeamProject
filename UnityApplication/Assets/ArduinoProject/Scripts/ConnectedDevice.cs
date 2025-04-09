using UnityEngine;

public class ConnectedDevice : MonoBehaviour
{
    public GameObject Device;

    private void OnEnable()
    {
        BluetoothManager.Instance.OnConnectionStatusChanged += OnConnectionChanged;
    }

    private void OnDisable()
    {
        BluetoothManager.Instance.OnConnectionStatusChanged -= OnConnectionChanged;
    }

    private void OnConnectionChanged(bool isConnected)
    {
        this.gameObject.SetActive(!isConnected);
    }   
}