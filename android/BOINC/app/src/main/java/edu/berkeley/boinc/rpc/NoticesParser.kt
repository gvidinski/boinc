/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */
package edu.berkeley.boinc.rpc

import android.util.Log
import android.util.Xml
import edu.berkeley.boinc.rpc.Notice
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class NoticesParser : BaseParser() {
    private var mNotice: Notice? = null
    val notices: MutableList<Notice> = mutableListOf()

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(NOTICE_TAG, ignoreCase = true)) {
            mNotice = Notice()
        } else { // primitive
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (mNotice != null) { // inside <notice>
                if (localName.equals(NOTICE_TAG, ignoreCase = true)) { // Closing tag
                    if (mNotice!!.seqno != -1) { // seqno is a must
                        notices.add(mNotice!!)
                    }
                    mNotice = null
                } else { // decode inner tags
                    if (localName.equals(Notice.Fields.SEQNO, ignoreCase = true)) {
                        mNotice!!.seqno = mCurrentElement.toString().toInt()
                    } else if (localName.equals(Notice.Fields.TITLE, ignoreCase = true)) {
                        mNotice!!.title = mCurrentElement.toString()
                    } else if (localName.equals(DESCRIPTION, ignoreCase = true)) {
                        mNotice!!.description = mCurrentElement.toString()
                    } else if (localName.equals(Notice.Fields.CREATE_TIME, ignoreCase = true)) {
                        mNotice!!.createTime = mCurrentElement.toDouble()
                    } else if (localName.equals(Notice.Fields.ARRIVAL_TIME, ignoreCase = true)) {
                        mNotice!!.arrivalTime = mCurrentElement.toDouble()
                    } else if (localName.equals(Notice.Fields.CATEGORY, ignoreCase = true)) {
                        mNotice!!.category = mCurrentElement.toString()
                        if (mNotice!!.category.equalsAny("server", "scheduler",
                                        ignoreCase = false)) {
                            mNotice!!.isServerNotice = true
                        }
                        if (mNotice!!.category == "client") {
                            mNotice!!.isClientNotice = true
                        }
                    } else if (localName.equals(Notice.Fields.LINK, ignoreCase = true)) {
                        mNotice!!.link = mCurrentElement.toString()
                    } else if (localName.equals(PROJECT_NAME, ignoreCase = true)) {
                        mNotice!!.projectName = mCurrentElement.toString()
                    }
                }
            }
            mElementStarted = false
        } catch (e: NumberFormatException) {
            Log.d("NoticesParser", "NumberFormatException " + localName + " "
                    + mCurrentElement.toString())
        }
    }

    companion object {
        const val NOTICE_TAG = "notice"
        @JvmStatic
        fun parse(rpcResult: String): List<Notice> {
            return try {
                val parser = NoticesParser()
                Xml.parse(rpcResult.replace("&", "&amp;"), parser)
                parser.notices
            } catch (e: SAXException) {
                Log.d("NoticesParser", "SAXException " + e.message + e.exception)
                emptyList()
            }
        }
    }
}
