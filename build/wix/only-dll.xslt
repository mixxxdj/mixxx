<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://schemas.microsoft.com/wix/2006/wi"
                xmlns:wix="http://schemas.microsoft.com/wix/2006/wi">

  <!-- This XSLT takes an heat generated wxs file, remove all files that are not dll files
       and removes every Directory (resulting in a not recursive Heat) -->

  <!-- strip all extraneous whitespace -->
  <xsl:strip-space  elements="*"/>

  <!-- Copy all attributes and elements to the output. -->
  <xsl:template match="@*|*">
      <xsl:copy>
          <xsl:apply-templates select="@*" />
          <xsl:apply-templates select="*" />
      </xsl:copy>
  </xsl:template>

  <xsl:output method="xml" indent="yes" />
  
  <!-- Create searches for the directories to remove. -->
  <xsl:key name="dir-search" match="wix:Directory" use="@Id" />
  <xsl:key name="compref-search" match="wix:Component[ancestor::wix:Directory]" use="@Id" />
  
  <!-- Remove directories. -->
  <xsl:template match="wix:Directory" />
  
  <!-- Remove DirectoryRefs (and their parent Fragments) referencing those directories. -->
  <xsl:template match="wix:Fragment[wix:DirectoryRef[key('dir-search', @Id)]]" />
  
  <!-- Remove Components referencing those directories. -->
  <xsl:template match="wix:Component[key('dir-search', @Directory)]" />
  <xsl:template match="wix:ComponentRef[key('compref-search', @Id)]" />
  
  <!-- Exclude all File elements that are not a .dll file -->
  <xsl:template match="wix:Component[not(contains(wix:File/@Source,'.dll'))]" />
  <xsl:template match="wix:ComponentRef[not(contains(@Id,'.dll'))]" />
  
  <!-- Exclude all empty Directories -->
  <xsl:template match="wix:Directory[not(contains(*/@Id, '.dll'))]" />
  
</xsl:stylesheet>