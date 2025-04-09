using UnityEngine;

public class ConnectedDevice : MonoBehaviour
{
    public GameObject Device;

    private void OnEnable()
    {
        BluetoothManager.OnConnectionStatusChanged += OnConnectionChanged;
    }

    private void OnDisable()
    {
        BluetoothManager.OnConnectionStatusChanged -= OnConnectionChanged;
    }

    private void OnConnectionChanged(bool isConnected)
    {
        this.gameObject.SetActive(!isConnected);
    }   
}