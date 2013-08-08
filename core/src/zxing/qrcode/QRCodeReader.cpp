// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  QRCodeReader.cpp
 *  zxing
 *
 *  Created by Christian Brunschen on 20/05/2008.
 *  Copyright 2008 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/qrcode/detector/Detector.h>
#include <zxing/NotFoundException.h>

#include <iostream>

namespace zxing {
	namespace qrcode {

		using namespace std;
		using zxing::NotFoundException;

		QRCodeReader::QRCodeReader() :decoder_() {
		}
		//TODO: see if any of the other files in the qrcode tree need tryHarder
		Ref<Result> QRCodeReader::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
			if (hints.containsFormat(BarcodeFormat::PURE_BARCODE)) {
				Ref<BitMatrix> bits = extractPureBits(image->getBlackMatrix());
				Detector detector(bits);
				Ref<DetectorResult> detectorResult(detector.detect(hints));
				ArrayRef< Ref<ResultPoint> > points (detectorResult->getPoints());
				Ref<DecoderResult> decoderResult(decoder_.decode(detectorResult->getBits()));
				Ref<Result> result(
						new Result(decoderResult->getText(), decoderResult->getRawBytes(), points, BarcodeFormat::QR_CODE));
				return result;
			}
			else {
				Detector detector(image->getBlackMatrix());
				Ref<DetectorResult> detectorResult(detector.detect(hints));
				ArrayRef< Ref<ResultPoint> > points (detectorResult->getPoints());
				Ref<DecoderResult> decoderResult(decoder_.decode(detectorResult->getBits()));
				Ref<Result> result(
						new Result(decoderResult->getText(), decoderResult->getRawBytes(), points, BarcodeFormat::QR_CODE));
				return result;
			}
		}

		QRCodeReader::~QRCodeReader() {
		}

		Decoder& QRCodeReader::getDecoder() {
			return decoder_;
		}

		Ref<BitMatrix> QRCodeReader::extractPureBits(Ref<BitMatrix> image)  {
			ArrayRef<int> leftTopBlack = image->getTopLeftOnBit();
			ArrayRef<int> rightBottomBlack = image->getBottomRightOnBit();

			int modSize = moduleSize(leftTopBlack, image);

			int top = leftTopBlack[1];
			int bottom = rightBottomBlack[1];
			int left = leftTopBlack[0];
			int right = rightBottomBlack[0];

			// Sanity check!
			if (left >= right || top >= bottom) {
    			throw NotFoundException("QRCodeReader::extractPureBits: left >= right || top >= bottom");
			}

			if (bottom - top != right - left) {
				// Special case, where bottom-right module wasn't black so we found something else in the last row
				// Assume it's a square, so use height as the width
				right = left + (bottom - top);
			}

			int matrixWidth = (right - left + 1) / modSize;
			int matrixHeight = (bottom - top + 1) / modSize;
			if (matrixWidth <= 0 || matrixHeight <= 0) {
    			throw NotFoundException("QRCodeReader::extractPureBits: matrixWidth <= 0 || matrixHeight <= 0");
			}
			if (matrixHeight != matrixWidth) {
    			throw NotFoundException("QRCodeReader::extractPureBits: matrixWidth != matrixHeight");
			}

			// Push in the "border" by half the module width so that we start
			// sampling in the middle of the module. Just in case the image is a
			// little off, this will help recover.
			int nudge = modSize >> 1;
			top += nudge;
			left += nudge;

			// But careful that this does not sample off the edge
			int nudgedTooFarRight = left + (int) ((matrixWidth - 1) * modSize) - (right - 1);
			if (nudgedTooFarRight > 0) {
				left -= nudgedTooFarRight;
			}
			int nudgedTooFarDown = top + (int) ((matrixHeight - 1) * modSize) - (bottom - 1);
			if (nudgedTooFarDown > 0) {
				top -= nudgedTooFarDown;
			}

			// Now just read off the bits
  			Ref<BitMatrix> bits(new BitMatrix(matrixWidth, matrixHeight));
			for (int y = 0; y < matrixHeight; y++) {
				int iOffset = top + y * modSize;
				for (int x = 0; x < matrixWidth; x++) {
					if (image->get(left + x * modSize, iOffset)) {
						bits->set(x, y);
					}
				}
			}
			return bits;
		}

		int QRCodeReader::moduleSize(ArrayRef<int> leftTopBlack, Ref<BitMatrix> image) {
			int x = leftTopBlack[0];
			int y = leftTopBlack[1];
			int width = image->getWidth();
			int height = image->getHeight();
			bool inBlack = true;
			int transition = 0;

			while (x < width && y < height) {
				if (inBlack != image->get(x, y)) {
					if (++transition == 5) {
						break;
					}
					inBlack = !inBlack;
				}
				x++;
				y++;
			}
			if (x == width || y == height)
    			throw NotFoundException("QRCodeReader::moduleSize: not found!");
			return (x - leftTopBlack[0]) / 7;
		}



	}
}
