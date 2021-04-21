using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SingletonSceneBehaviour : MonoBehaviour
{
    void Start() {
        SceneManager.LoadScene("MainScene", LoadSceneMode.Additive);
    }
}
