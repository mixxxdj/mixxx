<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet
  version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" version="4.0" encoding="iso-8859-1" indent="yes" />
<xsl:strip-space elements="*" />

<xsl:variable name="COMMA" select="','" />

<xsl:template match="/">
<html>
  <xsl:apply-templates />
</html>
</xsl:template>

<xsl:template match="skin">
<head>
<style type="text/css">
  <xsl:apply-templates select="Background" mode="bg" />
</style>
</head>
<body>
  <xsl:apply-templates />
</body>
</xsl:template>

<xsl:template name="position">
  <xsl:param name="position" />
  
  <xsl:variable name="top" select="substring-after( $position, $COMMA )" />
  <xsl:variable name="left" select="substring-before( $position, $COMMA )" />top: <xsl:value-of select="$top" />px;left: <xsl:value-of select="$left" />px;</xsl:template>

<xsl:template name="size">
  <xsl:param name="size" />

  <xsl:variable name="height" select="substring-after( $size, $COMMA )" />
  <xsl:variable name="width" select="substring-before( $size, $COMMA )" />width:<xsl:value-of select="$width" />px;height: <xsl:value-of select="$height" />px;</xsl:template>

<xsl:template name="image">
  <xsl:param name="position" />
  <xsl:param name="path" />

  <xsl:variable name="src">
    <xsl:choose>
      <xsl:when test="contains( $path, '%' )">
        <xsl:variable name="p1" select="substring-before( $path, '%' )" />
        <xsl:variable name="p2" select="substring-after( $path, '%' )" />
        <xsl:value-of select="$p1" /><xsl:value-of select="$p2" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$path" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <div>
    <xsl:attribute name="style">
      <xsl:call-template name="position">
        <xsl:with-param name="position" select="$position" />
      </xsl:call-template>
    </xsl:attribute>
    <img alt="">
    <xsl:attribute name="src"><xsl:value-of select="$src" /></xsl:attribute>
    </img>
  </div>
</xsl:template>

<xsl:template match="SliderComposed|XSliderComposed">
  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="Slider" />
  </xsl:call-template>

  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="Handle" />
  </xsl:call-template>
</xsl:template>

<xsl:template match="Knob|Knobx|Display">
  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="Path" />
  </xsl:call-template>
</xsl:template>

<xsl:template match="PushButton">
  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="State/Unpressed" />
  </xsl:call-template>
</xsl:template>

<xsl:template match="VuMeter">
  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="PathBack" />
  </xsl:call-template>

  <xsl:call-template name="image">
    <xsl:with-param name="position" select="Pos" />
    <xsl:with-param name="path" select="PathVu" />
  </xsl:call-template>
</xsl:template>

<xsl:template match="Overview|NumberPos|NumberRate|NumberBpm|Text|Visual">
  <div>
    <xsl:attribute name="style">
      <xsl:call-template name="position">
        <xsl:with-param name="position" select="Pos" />
      </xsl:call-template>
      <xsl:call-template name="size">
        <xsl:with-param name="size" select="Size" />
      </xsl:call-template><xsl:apply-templates /></xsl:attribute>
  </div>
</xsl:template>

<xsl:template match="Splitter">
  <div>
    <xsl:attribute name="style">
      <xsl:call-template name="position">
        <xsl:with-param name="position" select="Pos" />
      </xsl:call-template>
      <xsl:call-template name="size">
        <xsl:with-param name="size" select="Size" />
      </xsl:call-template>background: gray;</xsl:attribute>
  </div>
</xsl:template>

<xsl:template match="TreeView">
  <div>
    <xsl:attribute name="style">
      <xsl:call-template name="position">
        <xsl:with-param name="position" select="../Splitter/Pos" />
      </xsl:call-template>
      <xsl:call-template name="size">
        <xsl:with-param name="size" select="Size" />
      </xsl:call-template><xsl:apply-templates /></xsl:attribute>
  </div>
</xsl:template>

<xsl:template match="TrackTable"></xsl:template>

<xsl:template match="BgColor">background: <xsl:apply-templates />;</xsl:template>

<xsl:template match="FgColor">color: <xsl:apply-templates />;</xsl:template>

<xsl:template match="Background" mode="bg">
body {
  background-image: url( <xsl:value-of select="Path" /> );
  background-repeat: no-repeat;
  margin: 0px;
  padding: 0px;
}

div {
  position: absolute;
}
</xsl:template>

<xsl:template match="*"></xsl:template>

</xsl:stylesheet>
