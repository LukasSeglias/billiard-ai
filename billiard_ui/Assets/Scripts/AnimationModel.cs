using System;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

[Serializable]
public class RootObject
{
	public AnimationModel[] animations = new AnimationModel[0];
}

[Serializable]
public class AnimationModel
{
    public KeyFrame[] keyFrames;
}

[Serializable]
public class KeyFrame
{
    public double time;
    public Ball[] balls;
}

[Serializable]
public class Vec2 {
	public double x;
	public double y;
	
	public static Vec2 operator /(Vec2 vec, float value) => new Vec2{x = vec.x / value, y = vec.y / value};
}

[Serializable]
public class Ball
{
    public string type;
	public string id;
    public Vec2 position;
    public Vec2 velocity;
	public bool visible;
}

[Serializable]
public class RailSegment {
	public Vec2 start;
	public Vec2 end;
	
	public static RailSegment operator /(RailSegment segment, float value) => new RailSegment{
		start = segment.start / value,
		end = segment.end / value
	};
}
	
[Serializable]
public class Circle {
	public float radius;
	public Vec2 position;
	
	public static Circle operator /(Circle circle, float value) => new Circle{
		radius = circle.radius / value,
		position = circle.position / value
	};
}
	
[Serializable]
public class ArucoMarker {
	public Vec2 position;
	public float sideLength;
	
	public static ArucoMarker operator /(ArucoMarker marker, float value) => new ArucoMarker{
		position = marker.position / value,
		sideLength = marker.sideLength / value
	};
}

[Serializable]
public class Configuration {
	public float radius;
	public float width;
    public float height;
	public float scale;
	public RailSegment[] segments;
	public Circle[] targets;
	public ArucoMarker[] markers;
	public Correction correctionWidth;
	public Correction correctionHeight;
	
	public static Configuration operator /(Configuration config, float value) => new Configuration{
		radius = config.radius / value,
		width = config.width / value,
		height = config.height / value,
		scale = config.scale,
		segments = config.segments.Select(segment => segment / value).ToArray(),
		targets = config.targets.Select(target => target / value).ToArray(),
		markers = config.markers.Select(marker => marker / value).ToArray(),
		correctionWidth = new Correction{factor = config.correctionWidth.factor, translation = config.correctionWidth.translation},
		correctionHeight = new Correction{factor = config.correctionHeight.factor, translation = config.correctionHeight.translation}
	};
}

[Serializable]
public class Correction {
	public float factor;
	public float translation;
}

[Serializable]
public class Search
{
    public string id;
	public string type;
}

public class PickableInformation : MonoBehaviour
{
    public string id;
	public string type;
	public int picked = 0;
}