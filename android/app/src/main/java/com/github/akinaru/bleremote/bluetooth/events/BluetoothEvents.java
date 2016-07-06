/****************************************************************************
 * This file is part of Bluetooth LE Analyzer.                              *
 * <p/>                                                                     *
 * Copyright (C) 2016  Bertrand Martel                                      *
 * <p/>                                                                     *
 * Foobar is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation, either version 3 of the License, or        *
 * (at your option) any later version.                                      *
 * <p/>                                                                     *
 * Foobar is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 * <p/>                                                                     *
 * You should have received a copy of the GNU General Public License        *
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.          *
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
