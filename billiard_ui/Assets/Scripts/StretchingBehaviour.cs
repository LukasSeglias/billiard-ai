using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum FunctionMapping {
	FACTOR
}

public class StretchingBehaviour : MonoBehaviour
{
	public float Factor = 1.0f;
	public float Translation = 0.0f;
	public FunctionMapping Mapping = FunctionMapping.FACTOR;
	
	private MappingFunc mapping;
	
	private static readonly Dictionary<FunctionMapping, Func<float, float, MappingFunc>> MAPPING = new Dictionary<FunctionMapping, Func<float, float, MappingFunc>> {
		{FunctionMapping.FACTOR, StretchingBehaviour.initFactorMapping}
	};

	

    public void init(float value) {
		mapping = MAPPING[Mapping](value, Factor);
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
	
	private static MappingFunc initFactorMapping(float value, float factor) {
		Func<float, float> forwardMapping = (position) => {
			return (position * factor);
		};
		
		Func<float, float> inverseMapping = (position) => {
			return (position / factor);
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
