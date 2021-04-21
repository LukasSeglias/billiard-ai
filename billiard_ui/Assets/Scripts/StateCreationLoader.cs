using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class StateCreationLoader : MonoBehaviour
{
	public bool Enabled = false;

    // Update is called once per frame
    void Update()
    {
		
        if (Enabled && Input.GetKeyDown(KeyCode.S)) {
			SceneManager.UnloadSceneAsync("MainScene");
			SceneManager.LoadScene("StateCreationScene", LoadSceneMode.Additive);
		}
    }
}
