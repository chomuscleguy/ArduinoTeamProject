using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class CircularUI : MonoBehaviour
{
    public Image loadingImage;
    public float fillspeed = 1f;
    public float rotateSpeed = 60f;

    private float totalRotation = 0f;
    private float progress = 0f;
    private bool filling = true;
    private bool flipped = false;

    void Update()
    {
        if (filling)
        {
            progress += Time.deltaTime * fillspeed;
            if (progress >= 1f)
            {
                progress = 1f;
                filling = false;
                FlipImage();
            }
        }
        else
        {
            progress -= Time.deltaTime * fillspeed;
            if (progress <= 0f)
            {
                progress = 0f;
                filling = true;
                FlipImage();
            }
        }
        RotateCircle();

        loadingImage.fillAmount = progress;
    }

    private void RotateCircle()
    {
        totalRotation += Time.deltaTime * rotateSpeed;

        totalRotation %= 360f;

        loadingImage.rectTransform.rotation = Quaternion.Euler(0f, 0f, totalRotation);
    }
    void FlipImage()
    {
        flipped = !flipped;
        Vector3 scale = loadingImage.rectTransform.localScale;
        scale.x = Mathf.Abs(scale.x) * (flipped ? -1 : 1);
        loadingImage.rectTransform.localScale = scale;
    }

    public void Active()
    {
        var Devices = new List<DeviceData>(DataManager.Instance.deviceData.Values);

        foreach (DeviceData deviceData in Devices)
        {
            if (deviceData.Mac == this.name)
            {
                gameObject.SetActive(deviceData.connected);
                return;
            }
        }

        gameObject.SetActive(false);
    }
}