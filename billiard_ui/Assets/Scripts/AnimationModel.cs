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
public class Configuration {
	public float radius;
	public float width;
    public float height;
	public float scale;
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