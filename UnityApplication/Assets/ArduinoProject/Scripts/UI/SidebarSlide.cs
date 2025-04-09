using System.Collections;
using UnityEngine;

public class SidebarSlide : MonoBehaviour
{
    public float slideDuration = 0.5f;

    private RectTransform rectTransform;
    private Vector2 shownPosition = Vector2.zero;
    private Vector2 hiddenPosition;
    private bool isVisible = false;
    private float slideTimer = 0f;

    void Start()
    {
        rectTransform = GetComponent<RectTransform>();
        hiddenPosition = rectTransform.anchoredPosition;
        rectTransform.anchoredPosition = hiddenPosition;
    }

    public void OnSidebar()
    {
        isVisible = !isVisible;
        slideTimer = 0f;
        StopAllCoroutines();
        StartCoroutine(SlideSidebar(isVisible ? shownPosition : hiddenPosition));
    }

    public void OffSidebar()
    {
        if (isVisible)
        {
            isVisible = false;
            slideTimer = 0f;
            StopAllCoroutines();
            StartCoroutine(SlideSidebar(hiddenPosition));
        }
    }

    private IEnumerator SlideSidebar(Vector2 target)
    {
        Vector2 start = rectTransform.anchoredPosition;

        while (slideTimer < slideDuration)
        {
            slideTimer += Time.deltaTime;
            float t = slideTimer / slideDuration;
            rectTransform.anchoredPosition = Vector2.Lerp(start, target, t);
            yield return null;
        }

        rectTransform.anchoredPosition = target;
    }
}
