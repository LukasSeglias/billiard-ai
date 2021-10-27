using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using TMPro;
using UnityEngine.SceneManagement;

public class StretchingUtility {

	public float heightFactor = 0.0f;
	public float widthFactor  = 0.0f;
	public float translationX = 0.0f;
	public float translationY = 0.0f;
	public float scale = 1.0f;

	private static StretchingUtility instance;

	public static StretchingUtility get() {
	    if (instance == null) {
	        instance = new StretchingUtility();
	    }
	    return instance;
	}

    StretchingUtility() {
		Configuration config = ConfigurationLoader.load();

		scale = config.scale;

        widthFactor  = config.correctionWidth.factor;
        translationX = config.correctionWidth.translation;

        heightFactor = config.correctionHeight.factor;
        translationY = config.correctionHeight.translation;
    }

	public Vector3 convert(Vec2 vector, float z) {
		return new Vector3((float)vector.x, (float)vector.y, z);
	}
	
	public Vector3 position(Vector3 pos) {
		return new Vector3 {
            x = pos.x * widthFactor + translationX,
            y = pos.y * heightFactor + translationY,
		    z = pos.z
		};
	}
	
	public Vec2 invPosition(Vec2 pos) {
		return new Vec2 {
            x = ((float) pos.x - translationX) / widthFactor,
            y = ((float) pos.y - translationY) / heightFactor
		};
	}
	
}
