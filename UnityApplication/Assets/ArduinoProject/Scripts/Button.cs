using System.Collections;
using UnityEngine;
using UnityEngine.UI;

public class LEDButton : MonoBehaviour
{
    public Image bar;
    public RectTransform handle;
    public bool isOnLED = false;
    private bool isButtonActive = true;

    public void OnClickButton()
    {
        if (!isButtonActive)
            return;

        isButtonActive = false;

        if (!isOnLED)
        {
            bar.color = Color.green;

            Vector2 targetPosition = handle.position + new Vector3(80, 0, 0);
            StartCoroutine(MoveHandlePosition(targetPosition));

            isOnLED = true;
        }
        else
        {
            bar.color = Color.gray;

            Vector2 targetPosition = handle.position - new Vector3(80, 0, 0);
            StartCoroutine(MoveHandlePosition(targetPosition));

            isOnLED = false;
        }
    }
    IEnumerator MoveHandlePosition(Vector2 targetPosition)
    {
        Vector2 startPosition = handle.position;
        float elapsedTime = 0f;
        float moveDuration = 0.5f;

        while (elapsedTime < moveDuration)
        {
            handle.position = Vector2.Lerp(startPosition, targetPosition, elapsedTime / moveDuration);
            elapsedTime += Time.deltaTime;
            yield return null;
        }

        handle.position = targetPosition;
        isButtonActive = true;
    }
}
