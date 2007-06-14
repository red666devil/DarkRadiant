#include "Curve.h"

#include "stringio.h"

namespace entity {

	namespace {
		
		inline void PointVertexArray_testSelect(PointVertex* first, std::size_t count, 
			SelectionTest& test, SelectionIntersection& best) 
		{
			test.TestLineStrip(
			    VertexPointer(
			        reinterpret_cast<VertexPointer::pointer>(&first->vertex),
			        sizeof(PointVertex)
			    ),
			    IndexPointer::index_type(count),
			    best
			);
		}
		
		inline bool ControlPoints_parse(ControlPoints& controlPoints, const char* value) {
			StringTokeniser tokeniser(value, " ");

			std::size_t size;
			if (!string_parse_size(tokeniser.getToken(), size)) {
				return false;
			}

			if (size < 3) {
				return false;
			}
			controlPoints.resize(size);

			if (!string_equal(tokeniser.getToken(), "(")) {
				return false;
			}
			
			for (ControlPoints::iterator i = controlPoints.begin(); 
				 i != controlPoints.end(); 
				 ++i)
			{
				if (!string_parse_double(tokeniser.getToken(), (*i).x()) || 
					!string_parse_double(tokeniser.getToken(), (*i).y()) || 
					!string_parse_double(tokeniser.getToken(), (*i).z()))
				{
					return false;
				}
			}
			
			if (!string_equal(tokeniser.getToken(), ")")) {
				return false;
			}
			return true;
		}
		
	} // namespace

Curve::Curve(const Callback& boundsChanged) :
	_boundsChanged(boundsChanged)
{}

std::string Curve::getEntityKeyValue() {
	std::string value;
	
	if (!_controlPointsTransformed.empty()) {
		value = intToStr(_controlPointsTransformed.size()) + " (";
		for (ControlPoints::const_iterator i = _controlPointsTransformed.begin(); 
			 i != _controlPointsTransformed.end(); 
			 ++i)
		{
			value += " " + floatToStr(i->x()) + " " + 
					 floatToStr(i->y()) + " " + floatToStr(i->z()) + " ";
		}
		value += ")";
	}
	
	return value;
}

SignalHandlerId Curve::connect(const SignalHandler& curveChanged) {
	curveChanged();
	return _curveChanged.connectLast(curveChanged);
}

void Curve::disconnect(SignalHandlerId id) {
	_curveChanged.disconnect(id);
}

void Curve::testSelect(Selector& selector, SelectionTest& test, SelectionIntersection& best) {
	PointVertexArray_testSelect(
		&_renderCurve.m_vertices[0], 
		_renderCurve.m_vertices.size(), 
		test, 
		best
	);
}

void Curve::revertTransform() {
	_controlPointsTransformed = _controlPoints;
}

void Curve::freezeTransform() {
	_controlPoints = _controlPointsTransformed;
}

ControlPoints& Curve::getTransformedControlPoints() {
	return _controlPointsTransformed;
}

ControlPoints& Curve::getControlPoints() {
	return _controlPoints;
}

void Curve::renderSolid(Renderer& renderer, const VolumeTest& volume, 
	const Matrix4& localToWorld) const
{
	renderer.addRenderable(_renderCurve, localToWorld);
}

const AABB& Curve::getBounds() const {
	return _bounds;
}

bool Curve::isEmpty() const {
	return _renderCurve.m_vertices.size() == 0;
}

bool Curve::parseCurve(const std::string& value) {
	return ControlPoints_parse(_controlPoints, value.c_str());
}

void Curve::curveChanged() {
	// Recalculate the tesselation
	tesselate();

	// Recalculate bounds
    _bounds = AABB();
    for (ControlPoints::iterator i = _controlPointsTransformed.begin(); 
    	 i != _controlPointsTransformed.end(); 
    	 ++i)
	{
		_bounds.includePoint(*i);
	}

	// Notify the bounds changed observer
	_boundsChanged();
	
	// Emit the "curve changed" signal
	_curveChanged();
}

void Curve::curveKeyChanged(const std::string& value) {
	// Try to parse and check for validity
	if (value.empty() || !parseCurve(value)) {
		clearCurve();
	}
	
	// Assimilate the working set 
	_controlPointsTransformed = _controlPoints;
	
	// Do the tesselation and emit the signals
	curveChanged();
}

void Curve::appendControlPoints(unsigned int numPoints) {
	std::size_t size = _controlPoints.size();
	
	if (size < 1) {
		return;
	}
	
	// The coordinates of the penultimate point (can be 0,0,0)
	Vector3 penultimate = (size > 1) ? _controlPoints[size - 2] : Vector3(0,0,0);
	Vector3 ultimate = _controlPoints[size - 1];
	
	// Calculate the extrapolation vector
	Vector3 extrapolation = ultimate - penultimate;
	
	// greebo: TODO: Use std::vector for this instead of that crappy Array
	ControlPoints newPoints;
	newPoints.resize(size+1);
	
	// Copy the points from the source set
	ControlPoints::iterator target = newPoints.begin();
	for(ControlPoints::iterator source = _controlPoints.begin(); 
		source != _controlPoints.end(); 
		++source, ++target)
	{
		*target = *source; 
	}
	
	// The iterator points now to the last element, fill it
	*target = ultimate + extrapolation;
	
	// Replace the old control points with the larger set
	_controlPoints = newPoints;
	
	// Update the transformation working set
	_controlPointsTransformed = _controlPoints;
}

} // namespace entity
