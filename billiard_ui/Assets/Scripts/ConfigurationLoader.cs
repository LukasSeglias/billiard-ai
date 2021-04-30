using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ConfigurationLoader
{
	private static Configuration config;
    public static Configuration load() {
		if (config == null) {
			var json = Resources.Load<TextAsset>("configuration");
			config = JsonUtility.FromJson<Configuration>(json.text);
			AnimationService.setConfiguration(config);
			config = config / 1000;
		}
		
		return config;
	}
}
