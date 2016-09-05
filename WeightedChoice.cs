using UnityEngine;
using System.Collections;

public class WeightedChoice : MonoBehaviour {

    int GetRandomIndex(bool[] arr, int weight)
    {
        int total = 0;
        for (int i = 0; i < arr.Length; i++)
            total += arr[i] ? 1 : weight;

        int index = Random.Range(0,total);
        int choice = 0;
        total = 0;
        for (int i = 0; i < arr.Length; i++)
        {
            if (total <= index)
                choice = i;
            total += arr[i] ? 1 : weight;
        }
        return choice;
    }

    void RandomPickTest(int weight)
    {
        const int num = 78;
        bool[] bin =new bool[num];
        int collected = 0;
        int count = 0;

        while (collected < num)
        {
            int index = GetRandomIndex(bin, weight);//rand() % num;
            bin[index] = true;
            int coll = 0;
            for (int i = 0; i < num; i++)
                if (bin[i])
                    coll++;

            count++;
            if (coll > collected)
            {
                collected = coll;
                //printf("%i: %i\n", collected, count);
            }
        }
        Debug.Log("" + weight + ":" + count);
    }

    // Use this for initialization
    void Start () {
        int[] weights = { 1, 2, 3, 5, 10, 20 };
        foreach (int w in weights)
        {
            RandomPickTest(w);
        }
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
