package com.example.opusrx

import android.os.Bundle
import com.example.opusrx.R
import android.util.Log
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
  private val TAG = "MainActivity"
  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.activity_main)
    Log.d(TAG, "onCreate: debug message")
    Log.w(TAG, "onCreate: warning message")
    Log.e(TAG, "onCreate: error message")
  }
}
