using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Utility
{
	public const float LINE_WIDTH = 0.02f;

	public static void drawCircle(List<GameObject> objects, float radius, int segments, float lineWidth = LINE_WIDTH) {
		
		foreach (var gameObject in objects) {
			LineRenderer circleRenderer = getRenderer(gameObject, lineWidth);
			circleRenderer.material.color = Color.white;
			circleRenderer.positionCount = segments + 2;				
			CreatePoints (circleRenderer, radius, segments);			
		}
	}
	
	private static LineRenderer getRenderer(GameObject currentSelected, float lineWidth = LINE_WIDTH) {
		LineRenderer circleRenderer = currentSelected.GetComponent<LineRenderer>();
		if (circleRenderer != null) {
			circleRenderer.positionCount = 0;
			return circleRenderer;
		}
		circleRenderer = currentSelected.AddComponent<LineRenderer>();
		circleRenderer.material = new Material(Shader.Find("Hidden/Internal-Colored"));
		circleRenderer.startWidth = lineWidth;
		circleRenderer.endWidth = lineWidth;
		circleRenderer.useWorldSpace = false;
		return circleRenderer;
	}
	
	private static void CreatePoints (LineRenderer circleRenderer, float radius, int segments) {
        float x;
        float y;

        float angle = 0f;
		float angleStep = 360f / segments;

        for (int i = 0; i < (segments + 2); i++)
        {
            y = Mathf.Cos (Mathf.Deg2Rad * angle) * radius;
            x = Mathf.Sin (Mathf.Deg2Rad * angle) * radius;

            circleRenderer.SetPosition(i, new Vector3(x, y, -0.01f));

            angle += angleStep;
        }
    }
}
