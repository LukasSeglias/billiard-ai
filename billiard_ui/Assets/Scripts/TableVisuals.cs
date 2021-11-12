using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using TMPro;
using UnityEngine.SceneManagement;

public class TableVisuals : MonoBehaviour
{
	public GameObject Table;
	public bool show = false;
	private Color lineColor = Color.white;
// 	private Color lineColor = Color.black;

	private List<GameObject> railSegments;
	private List<GameObject> railSegmentNormals;
	private List<GameObject> targets;

    void Start() {
		Configuration config = ConfigurationLoader.load();

		Table.transform.localScale = new Vector3(
            (config.width * StretchingUtility.get().widthFactor) / 10.0f,
            1,
            (config.height * StretchingUtility.get().heightFactor) / 10.0f
        );

        Table.transform.position = new Vector3(
            StretchingUtility.get().translationX,
            StretchingUtility.get().translationY,
            0
        );

		railSegments = drawRailSegments(ref config);
		railSegmentNormals = drawRailSegmentNormals(ref config);
		targets = drawTargets(ref config);

        showTableAndRailsAndTargets(show);
    }

	public void toggleTableAndRailsAndTargets() {
		bool enabled = !Table.GetComponent<Renderer>().enabled;
		showTableAndRailsAndTargets(enabled);
	}

	public void showTableAndRailsAndTargets(bool enabled) {
	    Table.GetComponent<Renderer>().enabled = enabled;
        foreach(var segment in railSegments) {
            segment.GetComponent<LineRenderer>().enabled = enabled;
        }
// 		foreach(var segmentNormal in railSegmentNormals) {
//             segmentNormal.GetComponent<LineRenderer>().enabled = enabled;
//         }
        foreach(var target in targets) {
            target.GetComponent<LineRenderer>().enabled = enabled;
        }
	}

	private List<GameObject> drawRailSegments(ref Configuration config) {
		List<GameObject> railSegments = new List<GameObject>();
		int index = 0;
		foreach(var segment in config.segments) {
			GameObject lineObject = new GameObject(string.Format("RailSegment{0}", index++));
			LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
			lRend.enabled = false;
			lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
			lRend.startColor = lineColor;
			lRend.endColor = lineColor;
			lRend.startWidth = 0.02f;
			lRend.endWidth = 0.02f;
			lRend.SetPosition(0, StretchingUtility.get().position(convert(segment.start, -0.01f)));
			lRend.SetPosition(1, StretchingUtility.get().position(convert(segment.end, -0.01f)));
			lRend.sortingOrder = 0;
			railSegments.Add(lineObject);
		}
		return railSegments;
	}

	private List<GameObject> drawRailSegmentNormals(ref Configuration config) {
        List<GameObject> railSegmentNormals = new List<GameObject>();
        int index = 0;
        foreach(var segment in config.segments) {
            GameObject lineObject = new GameObject(string.Format("RailSegmentNormal{0}", index++));
            LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
            lRend.enabled = false;
            lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
            lRend.startColor = Color.red;
            lRend.endColor = Color.blue;
            lRend.startWidth = 0.02f;
            lRend.endWidth = 0.02f;
            Vec2 normal = segment.normal();
            Vec2 startPoint = segment.start + (0.5f * (segment.end - segment.start));
            Vec2 endPoint = startPoint + 0.1 * normal;
            lRend.SetPosition(0, StretchingUtility.get().position(convert(startPoint, -0.01f)));
            lRend.SetPosition(1, StretchingUtility.get().position(convert(endPoint, -0.01f)));
            lRend.sortingOrder = 0;
            railSegmentNormals.Add(lineObject);
        }
        return railSegmentNormals;
    }
	
	private List<GameObject> drawTargets(ref Configuration config) {
		List<GameObject> targets = new List<GameObject>();
		int index = 0;
		foreach(var target in config.targets) {
			GameObject lineObject = new GameObject(string.Format("Target{0}", index++));
			List<GameObject> lineObjectList = new List<GameObject>();
			lineObjectList.Add(lineObject);
			LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
			lRend.enabled = false;
			lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
			lRend.startColor = lineColor;
			lRend.endColor = lineColor;
			lRend.startWidth = 0.02f;
			lRend.endWidth = 0.02f;
			lRend.sortingOrder = 0;
			CreatePoints(lRend, 50, target);
			railSegments.Add(lineObject);
		}
		return targets;
	}
	
	private void CreatePoints (LineRenderer circleRenderer, int segments, Circle circle) {
        float x;
        float y;

        float angle = 0f;
		float angleStep = 360f / segments;
		circleRenderer.positionCount = segments + 2;

        for (int i = 0; i < (segments + 2); i++)
        {
			var movedPos = StretchingUtility.get().position(convert(circle.position, -0.01f));
            y = Mathf.Cos (Mathf.Deg2Rad * angle) * (circle.radius * StretchingUtility.get().scale) + movedPos.y;
            x = Mathf.Sin (Mathf.Deg2Rad * angle) * (circle.radius * StretchingUtility.get().scale) + movedPos.x;

            circleRenderer.SetPosition(i, new Vector3(x, y, -0.01f));

            angle += angleStep;
        }
    }

	public Vector3 convert(Vec2 vector, float z) {
        return new Vector3((float)vector.x, (float)vector.y, z);
    }
}
