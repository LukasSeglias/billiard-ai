using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum FunctionMapping {
	LINEAR
}

public class StretchingBehaviour : MonoBehaviour
{
	public float Factor = 1.0f;
	public float Translation = 0.0f;
	public FunctionMapping Mapping = FunctionMapping.LINEAR;
	
	private MappingFunc mapping;
	
	private static readonly Dictionary<FunctionMapping, Func<float, float, float, float, MappingFunc>> MAPPING = new Dictionary<FunctionMapping, Func<float, float, float, float, MappingFunc>> {
		{FunctionMapping.LINEAR, StretchingBehaviour.initLinearMapping}
	};

	

    public void init(float value) {
		float x_0 = - value / 2;
		float y_0 = x_0;
		float x_1 = value / 2;
		float y_1 = Factor * x_1;
		
		mapping = MAPPING[Mapping](x_0, y_0, x_1, y_1);
	}
	
	public float forward(float value) {
		if (mapping != null) {
			return mapping.forwardMapping(value) + Translation;
		}
		return value;
	}
	
	public float inverse(float value) {
		if (mapping != null) {
			return mapping.inverseMapping(value - Translation);
		}
		return value;
	}
	
	public float distance(float position) {
		return position - (forward(position) - Translation);
	}
	
	private static MappingFunc initLinearMapping(float x_0, float y_0, float x_1, float y_1) {
		float m = (y_1 - y_0) / (x_1 - x_0);
		float b = y_1 - m * x_1;
		Func<float, float> forwardMapping = (position) => {
			return position * m + b;
		};
		
		Func<float, float> inverseMapping = (position) => {
			return (position - b) / m;
		};
		
		return new MappingFunc(forwardMapping, inverseMapping);
	}
	
	private class MappingFunc {
		public readonly Func<float, float> forwardMapping;  
		public readonly Func<float, float> inverseMapping;
	
		public MappingFunc(Func<float, float> forwardMapping, Func<float, float> inverseMapping) {
			this.forwardMapping = forwardMapping;
			this.inverseMapping = inverseMapping;
		}
	}
}
