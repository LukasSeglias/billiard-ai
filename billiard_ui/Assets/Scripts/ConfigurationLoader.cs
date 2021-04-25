using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ConfigurationLoader
{
    public static Configuration load(ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching, ref GameObject table) {
		var json = Resources.Load<TextAsset>("configuration");
        var config = JsonUtility.FromJson<Configuration>(json.text);
		
		var height = config.height / 1000;
		var width = config.width / 1000;
		
		AnimationService.setConfiguration(config);
		heightStretching.Factor = config.correctionHeight.factor;
		heightStretching.Translation = config.correctionHeight.translation;
		heightStretching.init(height);
		
		widthStretching.Factor = config.correctionWidth.factor;
		widthStretching.Translation = config.correctionWidth.translation;
		widthStretching.init(width);
		
		float heightDistance = heightStretching.distance((float) height / 2);
		float widthDistance = widthStretching.distance((float) width / 2);
		table.transform.localScale = new Vector3((width - widthDistance) / 10.0f, 1, (height - heightDistance) / 10.0f);
		table.transform.position = new Vector3(-(widthDistance / 2.0f) + widthStretching.Translation, -(heightDistance / 2.0f)+ heightStretching.Translation, 0);
		
		return config;
	}
}
