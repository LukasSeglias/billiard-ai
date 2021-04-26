using System;
using System.Collections.Generic;
using UnityEngine;

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
}
	
[Serializable]
public class Circle {
	public float radius;
	public Vec2 position;
}
	
[Serializable]
public class ArucoMarker {
	public Vec2 position;
	public float sideLength;
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