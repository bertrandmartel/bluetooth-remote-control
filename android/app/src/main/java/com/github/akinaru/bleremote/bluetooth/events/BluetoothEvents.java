/********************************************************************************
 * The MIT License (MIT)                                                        *
 * <p/>                                                                         *
 * Copyright (c) 2016 Bertrand Martel                                           *
 * <p/>                                                                         *
 * Permission is hereby granted, free of charge, to any person obtaining a copy *
 * of this software and associated documentation files (the "Software"), to deal*
 * in the Software without restriction, including without limitation the rights *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    *
 * copies of the Software, and to permit persons to whom the Software is        *
 * furnished to do so, subject to the following conditions:                     *
 * <p/>                                                                         *
 * The above copyright notice and this permission notice shall be included in   *
 * all copies or substantial portions of the Software.                          *
 * <p/>                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,*
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    *
 * THE SOFTWARE.                                                                *
 */
package com.github.akinaru.bleremote.bluetooth.events;

/**
 * Filter action broadcasted from bluetooh custom service
 *
 * @author Bertrand Martel
 */
public class BluetoothEvents {

    public final static String BT_EVENT_DEVICE_DISCONNECTED = "com.github.akinaru.bledisplayremote.bluetooth.BT_EVENT_DEVICE_DISCONNECTED";
    public final static String BT_EVENT_DEVICE_CONNECTED = "com.github.akinaru.bledisplayremote.bluetooth.BT_EVENT_DEVICE_CONNECTED";
    public final static String BT_EVENT_SCAN_START = "com.github.akinaru.bledisplayremote.bluetooth.BT_EVENT_SCAN_START";
    public final static String BT_EVENT_SCAN_END = "com.github.akinaru.bledisplayremote.bluetooth.BT_EVENT_SCAN_END";
    public final static String BT_EVENT_DEVICE_DISCOVERED = "com.github.akinaru.bledisplayremote.bluetooth.BT_EVENT_DEVICE_DISCOVERED";

}
