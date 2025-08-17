package com.example.velicham

import android.hardware.camera2.CameraManager
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val inputField = findViewById<EditText>(R.id.inputField)
        val sendButton = findViewById<Button>(R.id.sendButton)
        val cameraManager = getSystemService(CAMERA_SERVICE) as CameraManager
        val cameraId = cameraManager.cameraIdList[0]  // Use back camera

        sendButton.setOnClickListener {
            val inputData = inputField.text.toString()
            if (inputData.length == 4 && inputData.all { it.isDigit() }) {
                flashData(inputData, cameraManager, cameraId)
            }
        }
    }

    private fun flashData(data: String, cameraManager: CameraManager, cameraId: String) {
        for (char in data) {
            val duration = 100 + (char.digitToInt() * 50)
            toggleFlashlight(cameraManager, cameraId, true)
            Thread.sleep(duration.toLong())
            toggleFlashlight(cameraManager, cameraId, false)
            Thread.sleep(50)  // Off time
        }
    }

    private fun toggleFlashlight(cameraManager: CameraManager, cameraId: String, state: Boolean) {
        cameraManager.setTorchMode(cameraId, state)
    }
}