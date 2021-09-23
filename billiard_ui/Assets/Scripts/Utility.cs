using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Utility
{
	
    public static void applyConfig(ref Configuration config, ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching, ref GameObject table) {		
		heightStretching.Factor = config.correctionHeight.factor;
		heightStretching.Translation = config.correctionHeight.translation;
		heightStretching.init(config.height);
		
		widthStretching.Factor = config.correctionWidth.factor;
		widthStretching.Translation = config.correctionWidth.translation;
		widthStretching.init(config.width);

		table.transform.localScale = new Vector3((config.width * widthStretching.Factor) / 10.0f, 1, (config.height * heightStretching.Factor) / 10.0f);
		table.transform.position = new Vector3(widthStretching.Translation, heightStretching.Translation, 0);
	}
	
	public static void drawCircle(List<GameObject> objects, float radius, int segments) {		
		
		foreach (var gameObject in objects) {
			LineRenderer circleRenderer = getRenderer(gameObject);
			circleRenderer.material.color = Color.white;
			circleRenderer.positionCount = segments + 2;				
			CreatePoints (circleRenderer, radius, segments);			
		}
	}
	
	private static LineRenderer getRenderer(GameObject currentSelected) {
		LineRenderer circleRenderer = currentSelected.GetComponent<LineRenderer>();
		if (circleRenderer != null) {
			circleRenderer.positionCount = 0;
			return circleRenderer;
		}
		circleRenderer = currentSelected.AddComponent<LineRenderer>();
		circleRenderer.material = new Material(Shader.Find("Hidden/Internal-Colored"));
		circleRenderer.startWidth = 0.02f;
		circleRenderer.endWidth = 0.02f;
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

            circleRenderer.SetPosition(i, new Vector3(x, y, 0));

            angle += angleStep;
        }
    }
}
