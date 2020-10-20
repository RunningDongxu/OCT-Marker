/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "wgsegnn.h"

#ifdef ML_SUPPORT

#include "wgsegmentation.h"
#include "bscansegmentation.h"
#include "bscanseglocalopnn.h"
#include "windownninout.h"

#include <helper/callback.h>

#include <QFileDialog>

#include 
#include<opencv2/opencv.hpp>


WgSegNN::WgSegNN(WGSegmentation* parent, BScanSegmentation* seg)
: QWidget(parent)
, segmentation(seg)
, localOpNN(seg->getLocalOpNN())
{
	setupUi(this);

	setNNData(BScanSegmentationMarker::NNTrainData());

	labelNumberExampels->setText("0");

	createConnections();
	updateActLayerInfo();
}


WgSegNN::~WgSegNN()
{
	delete inOutWindow;
}


void WgSegNN::createConnections()
{

	connect(buttonBoxLoadSave , &QDialogButtonBox::clicked, this, &WgSegNN::slotLoadSaveButtonBoxClicked);
	connect(pushButtonTrain   , &QAbstractButton ::clicked, this, &WgSegNN::slotTrain                   );
	connect(pbAddBscanExampels, &QAbstractButton ::clicked, this, &WgSegNN::slotAddBscanExampels        );
	connect(pbSetNNConfig     , &QAbstractButton ::clicked, this, &WgSegNN::changeNNConfig              );
	connect(btnShowInOutNN    , &QAbstractButton ::toggled, this, &WgSegNN::showInOutWindow              );
}


void WgSegNN::slotLoadSaveButtonBoxClicked(QAbstractButton* button)
{
	QDialogButtonBox::StandardButton stdButton = buttonBoxLoadSave->standardButton(button);

	switch(stdButton)
	{
		case QDialogButtonBox::Open:
			slotLoad();
			break;
		case QDialogButtonBox::Save:
			slotSave();
			break;
		default:
			break;
	}
}


void WgSegNN::getNNData(BScanSegmentationMarker::NNTrainData& data)
{
	data.maxIterations = sbMaxIterations->value();
	data.epsilon       = sbEpsilon      ->value();
/*
	data.learnRate     = sbLearnRate    ->value();
	data.momentScale   = sbMomentScale  ->value();

	data.delta0        = sbDelta0       ->value();
	data.deltaMax      = sbDeltaMax     ->value();
	data.deltaMin      = sbDeltaMin     ->value();
	data.nueMinus      = sbNueMinus     ->value();
	data.nuePlus       = sbNuePlus      ->value();

	if(rbBackPropagation->isChecked())
		data.trainMethod = BScanSegmentationMarker::NNTrainData::TrainMethod::Backpropergation;
	else
		data.trainMethod = BScanSegmentationMarker::NNTrainData::TrainMethod::RPROP           ;*/
}


void WgSegNN::setNNData(const BScanSegmentationMarker::NNTrainData& data)
{

	sbMaxIterations->setValue(static_cast<int>(data.maxIterations));
	sbEpsilon      ->setValue(data.epsilon      );
/*
	sbLearnRate    ->setValue(data.learnRate    );
	sbMomentScale  ->setValue(data.momentScale  );

	sbDelta0       ->setValue(data.delta0       );
	sbDeltaMax     ->setValue(data.deltaMax     );
	sbDeltaMin     ->setValue(data.deltaMin     );
	sbNueMinus     ->setValue(data.nueMinus     );
	sbNuePlus      ->setValue(data.nuePlus      );

	rbBackPropagation->setChecked(data.trainMethod == BScanSegmentationMarker::NNTrainData::TrainMethod::Backpropergation);
	rbRPROP          ->setChecked(data.trainMethod == BScanSegmentationMarker::NNTrainData::TrainMethod::RPROP           );*/
}



void WgSegNN::slotTrain()
{
	CallbackProgressDialog process("Learn", "Cancel");
	BScanSegmentationMarker::NNTrainData data;
	getNNData(data);
	localOpNN->trainNN(data, process);
}

void WgSegNN::slotLoad()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Load NN"), QString(), "*.fann");
	if(!file.isEmpty())
	{
		localOpNN->loadNN(file);
		updateActLayerInfo();
	}
}

void WgSegNN::slotSave()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save NN"), QString(), "*.fann");
	if(!file.isEmpty())
		localOpNN->saveNN(file);
}

void WgSegNN::slotAddBscanExampels()
{
	localOpNN->addBscanExampels();
	updateExampleInfo();
}

void WgSegNN::updateExampleInfo()
{
	labelNumberExampels->setText(QString("%1").arg(localOpNN->numExampels()));
}


void WgSegNN::updateActLayerInfo()
{
	const int inputHeight  = localOpNN->getInputHeight ();
	const int inputWidth   = localOpNN->getInputWidth  ();
	const int outputHeight = localOpNN->getOutputHeight();
	const int outputWidth  = localOpNN->getOutputWidth ();

	const std::vector<unsigned int> layers  = localOpNN->getLayerSizes();

	unsigned int inputNeurons  = 0;
	unsigned int outputNeurons = 0;
	std::size_t numLayers = layers.size();

	if(numLayers > 1)
	{
		inputNeurons  = layers[0];
		outputNeurons = layers[numLayers-1];
	}

	labelActInputSize ->setText(QString("%1 x %2 (%3)").arg(inputWidth ).arg(inputHeight ).arg(inputNeurons ));
	labelActOutputSize->setText(QString("%1 x %2 (%3)").arg(outputWidth).arg(outputHeight).arg(outputNeurons));

	QString layerString;
	for(std::size_t i = 1; i<numLayers-1; ++i)
	{
		if(i > 1)
			layerString += " ";
		layerString += QString("%1").arg(layers[i]);
	}
	labelActHiddenLayers->setText(layerString);
}


void WgSegNN::changeNNConfig()
{
	QString nnHiddenLayers = lineEditHiddenLayers->text();
	int inputHWidth  = spinBoxInputX->value();
	int inputHeight  = spinBoxInputY->value();
	int outputHWidth = spinBoxOutputX->value();
	int outputHeight = spinBoxOutputY->value();

	localOpNN->setNNConfig(nnHiddenLayers.toStdString(), inputHWidth, inputHeight, outputHWidth, outputHeight);

	updateActLayerInfo();
	updateExampleInfo();
}

void WgSegNN::showInOutWindow(bool show)
{
	class CallbackInOutNeuronsWindow : public BScanSegLocalOpNN::CallbackInOutNeurons
	{
		WindowNNInOut* inOutWindow = nullptr;
	public:
		void setWindow(WindowNNInOut* window) { inOutWindow = window; }
		virtual void processedInOutNeurons(const cv::Mat& in, const cv::Mat& out) const override
		{
			if(inOutWindow)
				inOutWindow->showInOutMat(in, out);
		}
	};

	static CallbackInOutNeuronsWindow callbackWindow;

	if(show)
	{
		if(!inOutWindow)
		{
			inOutWindow = new WindowNNInOut();
			callbackWindow.setWindow(inOutWindow);
			localOpNN->setCallbackInOutNeurons(&callbackWindow);
		}
		inOutWindow->show();
	}
	else
		inOutWindow->close();
}




#endif
