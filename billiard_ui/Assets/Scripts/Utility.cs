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
}
