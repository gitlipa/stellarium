/*
 * Copyright (C) 2016 Alexander Wolf
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

#ifndef CUSTOMOBJECTMGR_HPP
#define CUSTOMOBJECTMGR_HPP

#include "StelObjectModule.hpp"
#include "StelObject.hpp"
#include "StelTextureTypes.hpp"
#include "CustomObject.hpp"

#include <QFont>
#include <QList>

class StelPainter;
class QSettings;

typedef QSharedPointer<CustomObject> CustomObjectP;

class CustomObjectMgr : public StelObjectModule
{
	Q_OBJECT

public:
	CustomObjectMgr();
	~CustomObjectMgr() override;

	///////////////////////////////////////////////////////////////////////////
	// Methods defined in the StelModule class
	void init() override;
	void deinit() override;
	void draw(StelCore* core) override;
	virtual void drawPointer(StelCore* core, StelPainter& painter);
	double getCallOrder(StelModuleActionName actionName) const override;

	///////////////////////////////////////////////////////////////////////////
	// Methods defined in StelObjectModule class
	//! Used to get a list of objects which are near to some position.
	//! @param v a vector representing the position in the sky around which to search for objects.
	//! @param limitFov the field of view around the position v in which to search for objects.
	//! @param core the StelCore to use for computations.
	//! @return a list containing the objects located inside the limitFov circle around position v.
	QList<StelObjectP> searchAround(const Vec3d& v, double limitFov, const StelCore* core) const override;

	//! @return the matching object's pointer if exists or Q_NULLPTR.
	//! @param nameI18n The case in-sensitive localized name
	StelObjectP searchByNameI18n(const QString& nameI18n) const override;

	//! @return the matching object if exists or Q_NULLPTR.
	//! @param name The case in-sensitive english name
	StelObjectP searchByName(const QString& name) const override;

	StelObjectP searchByID(const QString &id) const override { return qSharedPointerCast<StelObject>(searchByEnglishName(id)); }

	QStringList listAllObjects(bool inEnglish) const override;
	QString getName() const override { return "Custom Objects"; }
	QString getStelObjectType() const override { return CustomObject::CUSTOMOBJECT_TYPE; }

	//! Handle mouse clicks. Please note that most of the interactions will be done through the GUI module.
	//! @return set the event as accepted if it was intercepted
	void handleMouseClicks(class QMouseEvent* e) override;

	//! Add persistent object on the sky
	//! @param designation - designation of custom object
	//! @param coordinates - coordinates of custom object
	void addPersistentObject(const QString& designation, Vec3d coordinates);

	//! Remove all persistent objects
	void removePersistentObjects();

public slots:
	///////////////////////////////////////////////////////////////////////////
	// Other public methods
	//! Get a pointer to a custom object.
	//! @param customObjectEnglishName the English name of the desired object.
	//! @return The matching custom object pointer if exists or Q_NULLPTR.
	CustomObjectP searchByEnglishName(QString customObjectEnglishName) const;

	//! Add custom object on the sky
	//! @param designation - designation of custom object
	//! @param coordinates - coordinates of custom object
	//! @param isVisible - flag of visibility of custom object
	void addCustomObject(const QString& designation, Vec3d coordinates, bool isVisible=false);
	//! Add custom object on the sky
	//! @param designation - designation of custom object
	//! @param ra - right ascension angle (J2000.0) of custom object
	//! @param dec - declination angle (J2000.0) of custom object
	//! @param isVisible - flag of visibility of custom object
	//! @code
	//! // example of usage in scripts
	//! CustomObjectMgr.addCustomObject("Marker", "2h10m15s", "60d01m15s", true);
	//! @endcode
	void addCustomObject(const QString& designation, const QString& ra, const QString& dec, bool isVisible=false);
	//! Add custom object on the sky
	//! @param designation - designation of custom object
	//! @param ra - right ascension angle (on date) of custom object
	//! @param dec - declination angle (on date) of custom object
	//! @param isVisible - flag of visibility of custom object
	//! @code
	//! // example of usage in scripts
	//! CustomObjectMgr.addCustomObjectRaDec("Marker", "2h10m15s", "60d01m15s", true);
	//! @endcode
	void addCustomObjectRaDec(const QString& designation, const QString& ra, const QString& dec, bool isVisible=false);
	//! Add custom object on the sky
	//! @param designation - designation of custom object
	//! @param alt - altitude of custom object
	//! @param azi - azimuth of custom object
	//! @param isVisible - flag of visibility of custom object
	//! @code
	//! // example of usage in scripts
	//! CustomObjectMgr.addCustomObjectAltAzi("Marker", "2d10m15s", "60d01m15s", true);
	//! @endcode
	void addCustomObjectAltAzi(const QString& designation, const QString& alt, const QString& azi, bool isVisible=false);
	//! Remove all custom objects
	void removeCustomObjects();
	//! Remove just one custom object by English name
	void removeCustomObject(QString englishName);

	//! Set the color used to draw custom object markers.
	//! @param c The color of the custom object markers (R,G,B)
	//! @code
	//! // example of usage in scripts (Qt6-based Stellarium)
	//! var c = new Color(1.0, 0.0, 0.0);
	//! CustomObjectMgr.setMarkersColor(c.toVec3f());
	//! @endcode
	void setMarkersColor(const Vec3f& c);
	//! Get the current color used to draw custom object markers.
	//! @return current color
	Vec3f getMarkersColor(void) const;

	//! Set the size of custom object markers.
	//! @param c The size of the custom object markers
	//! @code
	//! // example of usage in scripts
	//! CustomObjectMgr.setMarkersSize(5.f);
	//! @endcode
	void setMarkersSize(const float size);
	//! Get the current size used to custom object markers.
	//! @return current size
	float getMarkersSize(void) const;

	//! Set the select priority for custom objects
	//! @param priority level
	void setSelectPriority(float priority);

	//! Get the select priority for custom objects
	float getSelectPriority(void) const;

private slots:
	//! Called when a new object is selected.
	void selectedObjectChange(StelModule::StelModuleSelectAction action);

	//! Remove just one custom object
	void removeCustomObject(CustomObjectP);

	//! Connect this to StelApp font size.
	void setFontSize(int s){font.setPixelSize(s);}
private:
	// Font used for displaying our text
	QFont font;
	QSettings* conf;
	StelTextureSP texPointer;
	QList<CustomObjectP> customObjects, persistentObjects;

	Vec3f hightlightColor;
	int countMarkers;
	int radiusLimit;
	bool echoToLogfile;

	QString persistentCOFile;
	//! Loading list of saved custom objects (all of them are available in history of Search Tool)
	void loadPersistentObjects();
	//! Saving list of found via Search Tool custom objects
	void savePersistentObjects();

	//! Set the size of active radius around custom object markers.
	void setActiveRadiusLimit(const int radius);

	//! Set selected planets by englishName.
	//! @param englishName The custom object name or "" to select no object
	void setSelected(const QString& englishName);
	//! Set selected object from its pointer.
	void setSelected(CustomObjectP obj);
	//! Get selected object's pointer.
	CustomObjectP getSelected(void) const {return selected;}
	//! The currently selected planet.
	CustomObjectP selected;
};

#endif /* CUSTOMOBJECTMGR_HPP */
