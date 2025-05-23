/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau (some old code from the Planet class)
 * Copyright (C) 2010 Bogdan Marinov
 * Copyright (C) 2013-14 Georg Zotti (accuracy&speedup)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */
 
#include "MinorPlanet.hpp"
#include "Orbit.hpp"
#include "StelCore.hpp"
#include "StelTranslator.hpp"

#include <QRegularExpression>
#include <QDebug>
#include <QElapsedTimer>

MinorPlanet::MinorPlanet(const QString& englishName,
			 double radius,
			 double oblateness,
			 Vec3f halocolor,
			 float albedo,
			 float roughness,
			 const QString& atexMapName,
			 const QString& anormalMapName,
			 const QString& ahorizonMapName,
			 const QString& aobjModelName,
			 posFuncType coordFunc,
			 KeplerOrbit* orbitPtr,
			 OsculatingFunctType *osculatingFunc,
			 bool acloseOrbit,
			 bool hidden,
			 const QString &pTypeStr)
	: Planet (englishName,
		  radius,
		  oblateness,
		  halocolor,
		  albedo,
		  roughness,
		  atexMapName,
		  anormalMapName,
		  ahorizonMapName,
		  aobjModelName,
		  coordFunc,
		  orbitPtr,
		  osculatingFunc,
		  acloseOrbit,
		  hidden,
		  false, //No atmosphere
		  true,  //Halo
		  pTypeStr),
	minorPlanetNumber(0),
	slopeParameter(-10.0f), // -10 == mark as uninitialized: used in getVMagnitude()
	nameIsIAUDesignation(false),
	iauDesignationText(QString()),
	extraDesignations(),
	properName(englishName),
	b_v(99.f),
	specT(QString()),
	specB(QString())
{
	//Try to handle an occasional naming conflict between a moon and asteroid. Conflicting names are also shown with appended *.
	if (englishName.endsWith('*'))
		properName = englishName.left(englishName.length() - 1);

	//Try to detect IAU provisional designation
	QString iauDesignation = renderIAUDesignationinHtml(englishName);
	if (!iauDesignation.isEmpty())
	{
		nameIsIAUDesignation = true;
		iauDesignationHtml = iauDesignation;
	}
}

MinorPlanet::~MinorPlanet()
{
	//Do nothing for the moment
}

void MinorPlanet::setSpectralType(const QString &sT, const QString &sB)
{
	specT = sT;
	specB = sB;
}

void MinorPlanet::setColorIndexBV(float bv)
{
	b_v = bv;
}

void MinorPlanet::setMinorPlanetNumber(int number)
{
	if (minorPlanetNumber)
		return;

	minorPlanetNumber = number;
}

void MinorPlanet::setAbsoluteMagnitudeAndSlope(const float magnitude, const float slope)
{
	if ((slope < -1.0f) || (slope > 2.0f))
	{
		// G "should" be between 0 and 1, but may be somewhat outside.
		qWarning() << "MinorPlanet::setAbsoluteMagnitudeAndSlope(): Suspect slope parameter value" <<
			    QString::number(slope, 'f', 2) << "for" << getEnglishName() <<
			    ", expected between -1 and 2, mostly [0..1])";
		return;
	}
	absoluteMagnitude = magnitude;
	slopeParameter = slope;
}

void MinorPlanet::updateEquatorialRadius(void)
{
	if (absoluteMagnitude <= -99.f)
	{
		qWarning() << "MinorPlanet::updateEquatorialRadius():" <<
				"invalid absoluteMagnitude for" << getEnglishName();
		return;
	}
	if (equatorialRadius <= 0.)
	{
		if (albedo <= 0.0f)
		{
			// ESA NEOCC and NASA CNEOS assume albedo between 0.05 and 0.25
			// https://neo.ssa.esa.int/definitions-assumptions
			albedo = 0.15f;
		}
		// Estimate as described at http://www.physics.sfasu.edu/astro/asteroids/sizemagnitude.html
		float diameterKm = 1329.f / std::sqrt(albedo) * std::pow(10.f, -0.2f * absoluteMagnitude);
		equatorialRadius = 0.5 * diameterKm / AU;
	}
}

void MinorPlanet::setIAUDesignation(const QString &designation)
{
	//TODO: This feature has to be implemented better, anyway.
	if (!designation.isEmpty())
	{
		iauDesignationHtml = renderIAUDesignationinHtml(designation);
		iauDesignationText = designation;
		nameIsIAUDesignation = false;
	}
}

QString MinorPlanet::getEnglishName() const
{
	return (minorPlanetNumber ? QString("(%1) %2").arg(QString::number(minorPlanetNumber), englishName) : englishName);
}

QString MinorPlanet::getNameI18n() const
{
	return (minorPlanetNumber ? QString("(%1) %2").arg(QString::number(minorPlanetNumber), nameI18) : nameI18);
}

QString MinorPlanet::getInfoStringName(const StelCore *core, const InfoStringGroup& flags) const
{
	Q_UNUSED(core) Q_UNUSED(flags)
	QString str;
	QTextStream oss(&str);

	oss << "<h2>";
	if (nameIsIAUDesignation)
	{
		if (minorPlanetNumber)
			oss << QString("(%1) ").arg(minorPlanetNumber);
		oss << iauDesignationHtml;
	}
	else
		oss << getNameI18n();  // UI translation can differ from sky translation

	QStringList designations;
	if (!nameIsIAUDesignation && !iauDesignationHtml.isEmpty())
		designations << iauDesignationHtml;
	if (!getExtraDesignations().isEmpty())
		designations << getExtraDesignations();
	if (!designations.isEmpty())
		oss << QString(" (%1)").arg(designations.join(" - "));

	oss.setRealNumberNotation(QTextStream::FixedNotation);
	oss.setRealNumberPrecision(1);
	if (sphereScale != 1.)
		oss << QString::fromUtf8(" (\xC3\x97") << sphereScale << ")";

	oss << "</h2>";

	return str;
}

QString MinorPlanet::getInfoStringExtra(const StelCore *core, const InfoStringGroup& flags) const
{
	Q_UNUSED(core)
	QString str;
	QTextStream oss(&str);
	if (flags&Extra)
	{
		oss << Planet::getInfoStringExtra(core, flags);
		if (!specT.isEmpty())
		{
			// TRANSLATORS: Tholen spectral taxonomic classification of asteroids
			oss << QString("%1: %2<br/>").arg(q_("Tholen spectral type"), specT);
		}

		if (!specB.isEmpty())
		{
			// TRANSLATORS: SMASSII spectral taxonomic classification of asteroids
			oss << QString("%1: %2<br/>").arg(q_("SMASSII spectral type"), specB);
		}		
	}
	return str;
}

double MinorPlanet::getSiderealPeriod() const
{
	return static_cast<KeplerOrbit*>(orbitPtr)->calculateSiderealPeriod();
}

float MinorPlanet::getVMagnitude(const StelCore* core) const
{
	return getVMagnitude(core, 1.);
}
float MinorPlanet::getVMagnitude(const StelCore* core, const double eclipseFactor) const
{
	//If the H-G system is not used, use the default radius/albedo mechanism
	if (slopeParameter < -9.99f) // G can be somewhat <0! Set to -10 to mark invalid.
	{
		return Planet::getVMagnitude(core, eclipseFactor);
	}

	//Calculate phase angle
	//(Code copied from Planet::getVMagnitude())
	//(this is actually vector subtraction + the cosine theorem :))
	Vec3d observerHelioPos;
	if (core->getCurrentPlanet()->getPlanetType()==Planet::isObserver)

		observerHelioPos = Vec3d(0.f,0.f,0.f);
	else
		observerHelioPos = core->getObserverHeliocentricEclipticPos();
	const float observerRq = static_cast<float>(observerHelioPos.normSquared());
	const Vec3d& planetHelioPos = getHeliocentricEclipticPos();
	const float planetRq = static_cast<float>(planetHelioPos.normSquared());
	const float observerPlanetRq = static_cast<float>((observerHelioPos - planetHelioPos).normSquared());
	const float cos_chi = (observerPlanetRq + planetRq - observerRq)/(2.0f*std::sqrt(observerPlanetRq*planetRq));
	const float phaseAngle = std::acos(cos_chi);

	//Calculate reduced magnitude (magnitude without the influence of distance)
	//Source of the formulae: http://www.britastro.org/asteroids/dymock4.pdf
	// Same model as in Explanatory Supplement 2013, p.423
	const float tanPhaseAngleHalf=std::tan(phaseAngle*0.5f);
	const float phi1 = std::exp(-3.33f * std::pow(tanPhaseAngleHalf, 0.63f));
	const float phi2 = std::exp(-1.87f * std::pow(tanPhaseAngleHalf, 1.22f));
	float reducedMagnitude = absoluteMagnitude - 2.5f * std::log10( (1.0f - slopeParameter) * phi1 + slopeParameter * phi2 );

	//Calculate apparent magnitude
	float apparentMagnitude = reducedMagnitude + 5.0f * std::log10(std::sqrt(planetRq * observerPlanetRq));

	return apparentMagnitude;
}

void MinorPlanet::translateName(const StelTranslator &translator)
{
	nameI18 = translator.qtranslate(properName, "minor planet");
	if (englishName.endsWith('*'))
		nameI18.append('*');
}

QString MinorPlanet::renderIAUDesignationinHtml(const QString &plainTextName)
{
	static const QRegularExpression provisionalDesignationPattern("^(\\d{4}\\s[A-Z]{2})(\\d*)$");
	QRegularExpressionMatch match=provisionalDesignationPattern.match(plainTextName);
	if (plainTextName.indexOf(provisionalDesignationPattern) == 0)
	{
		QString main = match.captured(1);
		QString suffix = match.captured(2);
		if (!suffix.isEmpty())
			return (QString("%1<sub>%2</sub>").arg(main, suffix));
		else
			return main;
	}
	else
		return QString(); //plainTextName;
}

