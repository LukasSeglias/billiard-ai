using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ConfigurationLoader
{
    public static Configuration load(ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching, ref GameObject table) {
		var json = Resources.Load<TextAsset>("configuration");
        var config = JsonUtility.FromJson<Configuration>(json.text);
		
		AnimationService.setConfiguration(config);
		heightStretching.Factor = config.correctionHeight.factor;
		heightStretching.Translation = config.correctionHeight.translation;
		heightStretching.init(config.height);
		
		widthStretching.Factor = config.correctionWidth.factor;
		widthStretching.Translation = config.correctionWidth.translation;
		widthStretching.init(config.width);
		
		float heightDistance = heightStretching.distance((float) config.height / 2);
		float widthDistance = widthStretching.distance((float) config.width / 2);
		table.transform.localScale = new Vector3((config.width - widthDistance) / 10.0f, 1, (config.height - heightDistance) / 10.0f);
		table.transform.position = new Vector3(-(widthDistance / 2.0f) + widthStretching.Translation, -(heightDistance / 2.0f)+ heightStretching.Translation, 0);
		
		return config;
	}
}
